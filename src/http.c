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
  enum HttpStatusCode status_code;

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
static const char *http_head   = "HEAD";
static const char *http_post   = "POST";
static const char *http_delete = "DELETE";

static const char *htcpcp_status_i_am_a_teapot = HTCPCP_VERSION " 418 I'm a teapot";
static const char *htcpcp_brew = "BREW";

/********** Private APIs **********/

/**
 * Convert enumeration HttpMethod to string.
 *
 * @param method [in] HttpMethod to convert.
 * @return A string constant representing the HTTP method. The caller should not
 *         free the string!
 */
static const char *http_method_to_string(enum HttpMethod method)
{
  const char *ret = NULL;

  switch (method) {
    case HTTP_GET:
      ret = http_get;
      break;
    case HTTP_HEAD:
      ret = http_head;
      break;
    case HTTP_POST:
      ret = http_post;
      break;
    case HTTP_DELETE:
      ret = http_delete;
      break;
    case HTCPCP_BREW:
      ret = htcpcp_brew;
      break;
    default:
      break; // Remain ret to be NULL
  }

  return ret;
}

/**
 * Convert enumeration HttpStatusCode to string.
 *
 * @param method [in] HttpStatusCode to convert.
 * @return A string constant representing the HTTP status. The caller should not
 *         free the string!
 */
static const char *http_status_to_string(enum HttpStatusCode status)
{
  const char *ret = NULL;

  switch (status) {
    case HTTP_STATUS_OK:
      ret = http_status_ok;
      break;
    case HTTP_STATUS_NO_CONTENT:
      ret = http_status_no_content;
      break;

    case HTTP_STATUS_MOVED_PERMANENTLY:
      ret = http_status_moved_permanently;
      break;
    case HTTP_STATUS_FOUND:
      ret = http_status_found;
      break;

    case HTTP_STATUS_BAD_REQUEST:
      ret = http_status_bad_request;
      break;
    case HTTP_STATUS_FORBIDDEN:
      ret = http_status_forbidden;
      break;
    case HTTP_STATUS_NOT_FOUND:
      ret = http_status_not_found;
      break;
    case HTTP_STATUS_METHOD_NOT_ALLOWED:
      ret = http_status_method_not_allowed;
      break;

    case HTTP_STATUS_INTERNAL_SERVER_ERROR:
      ret = http_status_internal_server_error;
      break;

    case HTCPCP_STATUS_I_AM_A_TEAPOT:
      ret = htcpcp_status_i_am_a_teapot;
      break;

    default:
      break; // Remain ret to be NULL
  }
  return ret;
}

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
    size_t response_size = 0;

    // Convert the integer content_length into string
    char buffer[8]; // < For store the content-length
    snprintf(buffer, 8, "%d", response.content_length);

    // Calculate the buffer size to allocate
    response_size += strlen(response.version);
    response_size += strlen(http_status_to_string(response.status_code)) + strlen("\n");
    
    response_size += strlen("Content-Type: ") + strlen(response.content_type) + strlen("\n");
    response_size += strlen("Content-Length: ") + strlen(buffer) + strlen("\n");
    if (response.connection) {
      response_size += strlen("Connection: ") + strlen(response.connection) + strlen("\n");
    }
    if (response.location) {
      response_size += strlen("Location: ") + strlen(response.location) + strlen("\n");
    }
    if (response.allow) {
      response_size += strlen("Allow: ") + strlen(response.allow);
    }
    response_size += strlen("\r\n");
    
    if (response.content) {
      response_size += strlen(response.content) + strlen("\n");
    }

    char *output = (char*)malloc(sizeof(char) * response_size + 1);
    // NOTE: This is required for strcat() to work properly!
    output[0] = '\0';

    // The first line
    strcat(output, response.version);
    strcat(output, http_status_to_string(response.status_code));

    // Header
    strcat(output, "\nContent-Type: ");
    strcat(output, response.content_type);

    strcat(output, "\nContent-Length: ");
    strcat(output, buffer);

    if (response.connection) {
      strcat(output, "\nConnection: ");
      strcat(output, response.connection);
    }
    if (response.location) {
      strcat(output, "\nLocation: ");
      strcat(output, response.location);
    }
    if (response.allow) {
      strcat(output, "\nAllow: ");
      strcat(output, response.allow);
    }
    strcat(output, "\r\n");
    // Content
    if (response.content) {
      strcat(output, response.content);
      // strcat(output, "\0");
    }
    return output;
}

/********** Public APIs **********/

char *teapot_http_process(size_t *size, const char *input)
{
    // test case
    struct HttpResponse response =
    {
        HTTP_VERSION,
        HTTP_STATUS_OK,
        
        "text/plain",
        12,
        "close",
        NULL,
        NULL,
        
        "Hello world!",
    };
  
    char *response_str = teapot_http_response_construct(response);
    // char *response_str = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
    *size = strlen(response_str);// < Record the length of the response string
    return response_str;
}
