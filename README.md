# Teapot

Teapot is a simple HTTP server implemented in C.

## Build

```shell
make
```

The result binary will be in the current directory.

## Features

- Basic HTTP support
- Integrated TLS (HTTPS) support

## Dependencies

This program depends on:

- GIO (`gio-2.0`)
- GLib (`glib-2.0`)

The build system also uses `pkg-config` to discover dependencies.

## License

This project is licensed under the MIT license. See [COPYING](COPYING) for details.
