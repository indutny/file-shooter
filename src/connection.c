#include <stdlib.h>

#include "uv.h"
#include "openssl/ssl.h"

#include "src/connection.h"
#include "src/common.h"
#include "src/request.h"

static void fsh_connection_close_cb(uv_handle_t* handle) {
  fsh_connection_t* conn;

  conn = container_of(handle, fsh_connection_t, tcp);
  SSL_free(conn->ssl);
  conn->ssl = NULL;
  free(conn);
}


static void fsh_connection_link_close_cb(uv_link_t* link) {
  fsh_connection_t* conn;

  conn = container_of((uv_link_observer_t*) link, fsh_connection_t, observer);
  SSL_free(conn->ssl);
  conn->ssl = NULL;
  free(conn);
}


static void fsh_connection_read_cb(uv_link_observer_t* observer, ssize_t nread,
                                   const uv_buf_t* buf) {
  if (nread >= 0)
    return;

  uv_link_close((uv_link_t*) observer, fsh_connection_link_close_cb);
}


void fsh_connection_cb(uv_stream_t* server, int status) {
  fsh_connection_t* c;
  int err;

  CHECK(status);
  CHECK_ALLOC(c = malloc(sizeof(*c)));
  CHECK_ALLOC(c->ssl = SSL_new(secure_context));
  CHECK(uv_tcp_init(uv_default_loop(), &c->tcp));

  SSL_set_accept_state(c->ssl);

  if (uv_accept(server, (uv_stream_t*) &c->tcp) != 0)
    goto fail_accept;

  CHECK(uv_link_source_init(&c->source, (uv_stream_t*) &c->tcp));
  CHECK_ALLOC(c->ssl_link = uv_ssl_create(uv_default_loop(), c->ssl, &err));
  CHECK(err);
  CHECK_ALLOC(c->http = uv_http_create(fsh_request_cb, &err));
  CHECK(err);
  CHECK(uv_link_observer_init(&c->observer));

  /* Chain everything */
  CHECK(uv_link_chain((uv_link_t*) &c->source, (uv_link_t*) c->ssl_link));
  CHECK(uv_link_chain((uv_link_t*) c->ssl_link, (uv_link_t*) c->http));
  CHECK(uv_link_chain((uv_link_t*) c->http, (uv_link_t*) &c->observer));

  c->observer.observer_read_cb = fsh_connection_read_cb;

  /* Start handshake */
  CHECK(uv_link_read_start((uv_link_t*) &c->observer));

  return;

fail_accept:
  uv_close((uv_handle_t*) &c->tcp, fsh_connection_close_cb);
}
