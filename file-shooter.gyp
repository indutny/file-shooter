{
  "targets": [{
    "target_name": "file-shooter",
    "type": "executable",

    "include_dirs": [
      ".",
    ],

    "variables": {
      "gypkg_deps": [
        "git://github.com/libuv/libuv.git@^1.9.0 => uv.gyp:libuv",
        "git://github.com/indutny/uv_link_t@^1.0.0 => uv_link_t.gyp:uv_link_t",
        "git://github.com/indutny/uv_http_t@^1.0.0 => uv_http_t.gyp:uv_http_t",
        "git://github.com/indutny/uv_ssl_t@^1.0.0 => uv_ssl_t.gyp:uv_ssl_t",
        "git://github.com/gypkg/openssl@^1.2.7 => openssl.gyp:openssl",
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
