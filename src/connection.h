#ifndef SRC_CONNECTION_H_
#define SRC_CONNECTION_H_

#include "uv.h"
#include "uv_link_t.h"
#include "uv_ssl_t.h"
#include "uv_http_t.h"

typedef struct fsh_connection_s fsh_connection_t;

struct fsh_connection_s {
  uv_tcp_t tcp;
  uv_link_source_t source;
  uv_link_observer_t observer;

  SSL* ssl;
  uv_ssl_t* ssl_link;
  uv_http_t* http;
};

void fsh_connection_cb(uv_stream_t* server, int status);

#endif  /* SRC_CONNECTION_H_ */
