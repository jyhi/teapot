# Teapot

Teapot is a simple HTTP server implemented in C.

Disclaimer: This is a course project, and is far from finish. Do not use Teapot in serious scenario unless you know what you are doing.

## Build

```shell
make
```

The result binary will be in the current directory. The build system does not support shadow build.

## Usage

Run Teapot with `--help` to read all possible options. The following are the major ones:

- `-b` / `--bind` change binding address, default to 127.0.0.1
- `-p` / `--http-port` change HTTP service binding port, default to 8080
- `-P` / `--https-port` change HTTPS service binding port, default to 8443
- `-c` / `--cert` specify TLS certificate file, default to "cert.pem" in current directory
- `-k` / `--key` specify TLS private key file, default to "key.pem" in current directory

## Features

- Basic HTTP support
- Integrated TLS (HTTPS) support
- Quick reaction

## Dependencies

This program depends on:

- GIO (`gio-2.0`)
- GLib (`glib-2.0`)

The build system also uses `pkg-config` to discover dependencies.

## License

This project is licensed under the MIT license. See [COPYING](COPYING) for details.
