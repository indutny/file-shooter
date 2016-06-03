#include "uv_http_t.h"

#include "src/request.h"
#include "src/common.h"

static void fsh_request_close_cb(uv_link_t* link) {
  fsh_request_t* req;

  req = container_of(link, fsh_request_t, http);
  free(req);
}


static void fsh_request_shutdown_cb(uv_link_t* link, int status, void* arg) {
  uv_link_close(link, fsh_request_close_cb);
}


static void fsh_request_write_cb(uv_link_t* link, int status, void* arg) {
  /* No need for shutdown, request is not chunked */
  uv_link_close(link, fsh_request_close_cb);
}


void fsh_request_cb(uv_http_t* http, const char* url, size_t url_size) {
  fsh_request_t* req;
  fsh_file_t* file;
  uv_buf_t msg;
  unsigned short code;
  uv_buf_t fields[2];
  uv_buf_t values[ARRAY_SIZE(fields)];
  char tmp[128];
  int err;

  CHECK_ALLOC(req = malloc(sizeof(*req)));
  CHECK(uv_http_accept(http, &req->http));

  file = fsh_lookup_file(url, url_size);

  /* Bad Method */
  if (req->http.method != UV_HTTP_GET) {
    msg = uv_buf_init("Bad request", 11);
    code = 400;
  /* File not found */
  } else if (file == NULL) {
    msg = uv_buf_init("Not Found", 9);
    code = 404;
  } else {
    msg = uv_buf_init("OK", 2);
    code = 200;
  }

  if (code != 200) {
    err = uv_http_req_respond(&req->http, code, &msg, NULL, NULL, 0);
    if (err != 0)
      goto fail;

    err = uv_link_shutdown((uv_link_t*) &req->http, fsh_request_shutdown_cb,
                           NULL);
    if (err != 0)
      goto fail;
    return;
  }

  fields[0] = uv_buf_init("Content-Length", 14);
  values[0] = uv_buf_init(tmp,
      snprintf(tmp, sizeof(tmp), "%d", (int) file->content.len));
  fields[1] = uv_buf_init("Content-Type", 12);
  values[1] = file->type;

  req->http.chunked = 0;
  err = uv_http_req_respond(&req->http, code, &msg, fields, values,
                            ARRAY_SIZE(fields));
  if (err != 0)
    goto fail;

  err = uv_link_write((uv_link_t*) &req->http, &file->content, 1, NULL,
                      fsh_request_write_cb, NULL);
  if (err != 0)
    goto fail;

  return;

fail:
  uv_link_close((uv_link_t*) &req->http, fsh_request_close_cb);
}
