#include <glib.h>
#include <stdlib.h>
#include <stdint.h>
#include "file.h"

/********** Internal types **********/

/**
 * HTTP method in an HTTP request.
 */
enum HttpMethod {
  HTTP_METHOD_UNKNOWN, ///< Unknown verb, used to indicate a non-HTTP message
  HTTP_GET,            ///< HTTP GET request
  HTTP_HEAD,           ///< HTTP HEAD request
  HTTP_POST,           ///< HTTP POST request
  HTTP_DELETE,         ///< HTTP DELETE request

  HTCPCP_BREW,         ///< HTCPCP BREW request (I'm a teapot!)
};

/**
 * HTTP status code in an HTTP response.
 */
enum HttpStatusCode {
  HTTP_STATUS_UNKNOWN,                ///< Unknown HTTP status code
  HTTP_STATUS_OK,                     ///< HTTP 200
  HTTP_STATUS_NO_CONTENT,             ///< HTTP 204

  HTTP_STATUS_MOVED_PERMANENTLY,      ///< HTTP 301
  HTTP_STATUS_FOUND,                  ///< HTTP 302

  HTTP_STATUS_BAD_REQUEST,            ///< HTTP 400
  HTTP_STATUS_FORBIDDEN,              ///< HTTP 403
  HTTP_STATUS_NOT_FOUND,              ///< HTTP 404
  HTTP_STATUS_METHOD_NOT_ALLOWED,     ///< HTTP 405

  HTTP_STATUS_INTERNAL_SERVER_ERROR,  ///< HTTP 500

  HTCPCP_STATUS_I_AM_A_TEAPOT,        ///< HTCPCP 418 :)
};

/**
 * An HTTP request entity.
 */
struct HttpRequest {
  // Request line
  enum HttpMethod method;
  char *path;
  char *version;

  // Header fields
  char *host;
  char *content_type;
  size_t content_length;
  char *expect;

  // Content
  uint8_t *content;
};

/**
 * An HTTP response entity.
 */
struct HttpResponse {
  // Status line
  char *version;
  enum HttpStatusCode;

  // Header fields
  char *content_type;
  size_t content_length;
  char *connection;
  char *location;
  char *allow;

  // Content
  uint8_t *content;
};

/********** Internal states (variables) **********/

/**
 * HTTP version. This implementation sticks to HTTP/1.1, so there is no way to
 * change this string...
 */
#define HTTP_VERSION "HTTP/1.1"

/**
 * Hyper Text Coffee Pot Control Protocol version.
 */
#define HTCPCP_VERSION "HTCPCP/1.0"

/* String constants of HTTP contents for internal use */

static const char *http_status_ok         = HTTP_VERSION " 200 OK";
static const char *http_status_no_content = HTTP_VERSION " 204 No Content";

static const char *http_status_moved_permanently = HTTP_VERSION " 301 Moved Permanently";
static const char *http_status_found             = HTTP_VERSION " 302 Found";

static const char *http_status_bad_request            = HTTP_VERSION " 400 Bad Request";
static const char *http_status_forbidden              = HTTP_VERSION " 403 Forbidden";
static const char *http_status_not_found              = HTTP_VERSION " 404 Not Found";
static const char *http_status_method_not_allowed     = HTTP_VERSION " 405 Method Not Allowed";

static const char *http_status_internal_server_error = HTTP_VERSION " 500 Server Internal Error";

static const char *http_get    = "GET";
static const char *http_put    = "PUT";
static const char *http_post   = "POST";
static const char *http_delete = "DELETE";

static const char *htcpcp_status_i_am_a_teapot = HTCPCP_VERSION " 418 I'm a teapot";
static const char *htcpcp_brew = "BREW";

/********** Private APIs **********/

/**
 * Parse an HTTP string into `struct HttpRequest` for future processing.
 *
 * @param http [in] The HTTP string.
 * @return A `struct HttpRequest`.
 */
static struct HttpRequest teapot_http_request_parse(const char *http)
{
  // unimplemented
}

/**
 * Convert a `struct HttpResponse` to an HTTP string for sending.
 *
 * @param response [in] The `struct HttpResponse` to convert.
 * @return A NUL-terminated string of HTTP response.
 */
static char *teapot_http_response_construct(const struct HttpResponse response)
{
  // unimplemented
}

/********** Public APIs **********/

char *teapot_http_process(size_t *size, const char *input)
{
  // unimplemented
}
