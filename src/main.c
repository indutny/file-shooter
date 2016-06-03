#include <stdio.h>
#include <stdlib.h>

#include "openssl/ssl.h"
#include "openssl/bio.h"
#include "openssl/err.h"
#include "openssl/evp.h"
#include "openssl/pem.h"
#include "openssl/x509.h"

#include "uv.h"

#include "src/connection.h"
#include "src/common.h"

SSL_CTX* secure_context;
static fsh_file_t* files;
static uv_tcp_t server;


static void fsh_init_dhe(SSL_CTX* ctx) {
  int nid;
  EC_KEY* ecdh;

  nid = OBJ_sn2nid("prime256v1");
  if (nid == NID_undef)
    abort();

  ecdh = EC_KEY_new_by_curve_name(nid);
  if (ecdh == NULL)
    abort();

  SSL_CTX_set_tmp_ecdh(ctx, ecdh);
  EC_KEY_free(ecdh);
}


#define BUF(str) uv_buf_init(str, sizeof(str) - 1)


static uv_buf_t fsh_content_type(const char* path, int len) {
  if (len >= 5 && strncmp(path + len - 5, ".html", 5) == 0)
    return BUF("text/html");
  if (len >= 4 && strncmp(path + len - 4, ".css", 4) == 0)
    return BUF("text/css");
  if (len >= 4 && strncmp(path + len - 4, ".png", 4) == 0)
    return BUF("image/png");
  if (len >= 4 && strncmp(path + len - 4, ".svg", 4) == 0)
    return BUF("image/svg+xml");
  if (len >= 3 && strncmp(path + len - 3, ".js", 3) == 0)
    return BUF("application/javascript");

  return BUF("text/plain");
}


#undef BUF


static uv_buf_t fsh_content(const char* path) {
  uv_fs_t req;
  uv_file fd;
  uv_stat_t* stat;
  uv_buf_t res;
  int bytes;
  char tmp[4096];

  snprintf(tmp, sizeof(tmp), "public/%s", path);
  fd = uv_fs_open(uv_default_loop(), &req, tmp, O_RDONLY, 0, NULL);
  if (fd == -1)
    abort();

  CHECK(uv_fs_fstat(uv_default_loop(), &req, fd, NULL));
  stat = (uv_stat_t*) req.ptr;
  res.len = stat->st_size;
  uv_fs_req_cleanup(&req);

  CHECK_ALLOC(res.base = malloc(res.len));
  bytes = uv_fs_read(uv_default_loop(), &req, fd, &res, 1, 0, NULL);
  if (bytes < 0)
    abort();
  res.len = bytes;

  CHECK(uv_fs_close(uv_default_loop(), &req, fd, NULL));

  return res;
}


static void fsh_init_files() {
  uv_fs_t req;
  int count;
  int limit;

  limit = uv_fs_scandir(uv_default_loop(), &req, "public", 0, NULL);
  if (limit < 0)
    abort();

  count = 0;
  limit++;
  CHECK_ALLOC(files = malloc(sizeof(*files) * limit));

  for (;;) {
    int err;
    uv_dirent_t ent;
    fsh_file_t* file;

    err = uv_fs_scandir_next(&req, &ent);
    if (err == UV_EOF)
      break;
    if (err != 0)
      abort();

    if (ent.type != UV_DIRENT_FILE)
      continue;

    file = &files[count];

    CHECK_ALLOC(file->path = strdup(ent.name));
    file->path_len = strlen(file->path);

    file->type = fsh_content_type(file->path, file->path_len);
    file->content = fsh_content(file->path);

    count++;
    if (count > limit - 1)
      abort();
  }
  uv_fs_req_cleanup(&req);

  /* Last element */
  files[count].path = NULL;
}


fsh_file_t* fsh_lookup_file(const char* path, size_t path_len) {
  fsh_file_t* f;

  if (path_len < 1)
    return NULL;
  path_len--;
  path++;

  if (path_len == 0) {
    path = "index.html";
    path_len = 10;
  }

  f = files;
  for (; f->path != NULL; f++)
    if (f->path_len == path_len && memcmp(f->path, path, path_len) == 0)
      return f;

  return NULL;
}


int main() {
  static const int kBacklog = 128;

  uv_loop_t* loop;
  struct sockaddr_in6 addr;

  SSL_library_init();
  OpenSSL_add_all_algorithms();
  OpenSSL_add_all_digests();
  SSL_load_error_strings();
  ERR_load_crypto_strings();

  /* Initialize SSL_CTX */
  CHECK_ALLOC(secure_context = SSL_CTX_new(SSLv23_method()));

  SSL_CTX_use_certificate_file(secure_context, "keys/cert.pem",
                               SSL_FILETYPE_PEM);
  SSL_CTX_use_PrivateKey_file(secure_context, "keys/key.pem",
                              SSL_FILETYPE_PEM);

  /* Some secure configuration */
  SSL_CTX_set_options(secure_context, SSL_OP_NO_SSLv2);
  SSL_CTX_set_options(secure_context, SSL_OP_NO_SSLv3);
  SSL_CTX_set_session_cache_mode(secure_context,
                                 SSL_SESS_CACHE_SERVER |
                                 SSL_SESS_CACHE_NO_INTERNAL |
                                 SSL_SESS_CACHE_NO_AUTO_CLEAR);
  SSL_CTX_set_options(secure_context, SSL_OP_SINGLE_ECDH_USE);
  SSL_CTX_set_options(secure_context, SSL_OP_SINGLE_DH_USE);

  fsh_init_dhe(secure_context);
  fsh_init_files();

  loop = uv_default_loop();

  CHECK(uv_tcp_init(loop, &server));
  CHECK(uv_ip6_addr("::", 9000, &addr));
  CHECK(uv_tcp_bind(&server, (struct sockaddr*) &addr, 0));

  fprintf(stderr, "Listening on [::]:9000\n");

  CHECK(uv_listen((uv_stream_t*) &server, kBacklog, fsh_connection_cb));
  CHECK(uv_run(loop, UV_RUN_DEFAULT));

  return 0;
}
