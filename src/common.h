#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include <stdlib.h>

#include "openssl/ssl.h"

typedef struct fsh_file_s fsh_file_t;

struct fsh_file_s {
  const char* path;
  size_t path_len;

  uv_buf_t type;
  uv_buf_t content;
};

#define CHECK(V) if ((V) != 0) abort()
#define CHECK_ALLOC(V) if ((V) == NULL) abort()

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define container_of(ptr, type, member) \
    ((type *) ((char *) (ptr) - offsetof(type, member)))

SSL_CTX* secure_context;

fsh_file_t* fsh_lookup_file(const char* path, size_t path_len);

#endif  /* SRC_COMMON_H_ */
