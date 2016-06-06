{
  "targets": [{
    "target_name": "file-shooter",
    "type": "executable",

    "include_dirs": [
      ".",
    ],

    "variables": {
      "gypkg_deps": [
        "git://github.com/libuv/libuv.git#v1.9.1:uv.gyp:libuv",
        "git://github.com/indutny/uv_link_t:uv_link_t.gyp:uv_link_t",
        "git://github.com/indutny/uv_http_t:uv_http_t.gyp:uv_http_t",
        "git://github.com/indutny/uv_ssl_t:uv_ssl_t.gyp:uv_ssl_t",
        "git://github.com/indutny/bud:deps/openssl/openssl.gyp:openssl",
      ],
    },

    "dependencies": [
      "<!@(gypkg deps <(gypkg_deps))",
    ],

    "sources": [
      "src/main.c",
      "src/connection.c",
      "src/request.c",
    ],
  }],
}
