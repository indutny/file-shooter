{
  "targets": [{
    "target_name": "file-shooter",
    "type": "executable",

    "include_dirs": [

      ".",
    ],

    "dependencies": [
      "deps/libuv/uv.gyp:libuv",
      "deps/openssl/openssl.gyp:openssl",
      "deps/uv_link_t/uv_link_t.gyp:uv_link_t",
      "deps/uv_ssl_t/uv_ssl_t.gyp:uv_ssl_t",
      "deps/uv_http_t/uv_http_t.gyp:uv_http_t",
    ],

    "sources": [
      "src/main.c",
      "src/connection.c",
      "src/request.c",
    ],
  }],
}
