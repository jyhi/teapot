#ifndef TEAPOT_CONFIG_H
#define TEAPOT_CONFIG_H

/**
 * Define name string of Teapot.
 */
#define TEAPOT_NAME "Teapot"

/**
 * Define version string of Teapot.
 */
#define TEAPOT_VERSION "0.1"

/**
 * Define default binding address of Teapot.
 */
#define TEAPOT_DEFAULT_BIND_ADDRESS "127.0.0.1"

/**
 * Define default binding ports of Teapot.
 */
#define TEAPOT_DEFAULT_HTTP_PORT  8080
#define TEAPOT_DEFAULT_HTTPS_PORT 8443

/**
 * Define default path for TLS certificate and private key.
 */
#define TEAPOT_DEFAULT_TLS_CERTIFICATE_PATH "cert.pem"
#define TEAPOT_DEFAULT_TLS_PRIVATE_KEY_PATH "key.pem"

#endif
