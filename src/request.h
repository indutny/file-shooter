#ifndef SRC_REQUEST_H_
#define SRC_REQUEST_H_

#include "uv_http_t.h"

typedef struct fsh_request_s fsh_request_t;

struct fsh_request_s {
  uv_http_req_t http;
};

void fsh_request_cb(uv_http_t* http, const char* url, size_t url_size);

#endif  /* SRC_REQUEST_H_ */
