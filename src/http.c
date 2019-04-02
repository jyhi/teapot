#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "file.h"
#include "redir.h"
#include "http.h"

#define BUFSIZE 4096

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
 * HTTP header indicator, mainly for use in http_extract_header().
 */
enum RequestHeader {
  HTTP_HOST,                          ///< "Host: "
  HTTP_CONTENT_TYPE,                  ///< "Content-Type: "
  HTTP_HEADER_CONTENT_LENGTH,         ///< "Content-Length: "
  HTTP_HEADER_EXPECT,                 ///< "Expect: "
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
static const char *http_head    = "HEAD";
static const char *http_post   = "POST";
static const char *http_delete = "DELETE";

static const char *htcpcp_status_i_am_a_teapot = HTCPCP_VERSION " 418 I'm a teapot";
static const char *htcpcp_brew = "BREW";

static const char *http_header_host                 = "Host: ";
static const char *http_header_content_type         = "Content-Type: ";
static const char *http_header_content_length       = "Content-Length: ";
static const char *http_header_expect               = "Expect: ";

/********** Private APIs **********/

/**
 * Convert string into integer.
 */
static int toInteger(char a[]) {
  int c = '\0', sign = 0, offset = 0, n = 0;

  if (a[0] == '-') sign = -1;

  if (sign == -1) offset = 1;
  else offset = 0;

  n = 0;
  for (c = offset; a[c] != '\0'; c++) {
    n = n * 10 + a[c] - '0';
  }

  if (sign == -1) n = -n;
  return n;
}

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
    case HTTP_METHOD_UNKNOWN: // fall through
    // default: // <- clang thinks this is unnecessary...
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

    case HTTP_STATUS_UNKNOWN: // fall through
    // default:
      break; // Remain ret to be NULL
  }
  return ret;
}

static enum HttpMethod http_extract_request_method(const char *req)
{
  if (!req)
    return HTTP_METHOD_UNKNOWN;

  enum HttpMethod ret = HTTP_METHOD_UNKNOWN;

  if (g_str_has_prefix(req, "GET"))
    ret = HTTP_GET;
  else if (g_str_has_prefix(req, "HEAD"))
    ret = HTTP_HEAD;
  else if (g_str_has_prefix(req, "POST"))
    ret = HTTP_POST;
  else if (g_str_has_prefix(req, "DELETE"))
    ret = HTTP_DELETE;

  return ret;
}

static char *http_extract_request_path(const char *req)
{
  if (!req)
    return NULL;

  char *buffer = g_malloc(BUFSIZE);

  sscanf(req, "%*s %4095s", buffer); // XXX: Hardcoded buffer size

  char *ret = g_strdup(buffer);

  g_free(buffer);
  return ret;
}

static char *http_extract_request_version(const char *req)
{
  if (!req)
    return NULL;

  char *buffer = g_malloc(BUFSIZE);

  sscanf(req, "%*s %*s %4095s", buffer); // XXX: Hardcoded buffer size

  char *ret = g_strdup(buffer);

  g_free(buffer);
  return ret;
}

static char *http_extract_header(const char *http, enum RequestHeader header)
{
  if (!http)
    return NULL;

  char *buffer = g_malloc(BUFSIZE);
  const char *line = NULL;

  switch (header) {
    case HTTP_HOST:
      line = g_strstr_len(http, -1, http_header_host);
      if (line) {
        sscanf(line, "%*s %4095s", buffer);
      }
      break;
    case HTTP_CONTENT_TYPE:
      line = g_strstr_len(http, -1, http_header_content_type);
      if (line) {
        sscanf(line, "%*s %4095s", buffer);
      }
      break;
    case HTTP_HEADER_CONTENT_LENGTH:
      line = g_strstr_len(http, -1, http_header_content_length);
      if (line) {
        sscanf(line, "%*s %4095s", buffer);
      }
      break;
    case HTTP_HEADER_EXPECT:
      line = g_strstr_len(http, -1, http_header_expect);
      if (line) {
        sscanf(line, "%*s %4095s", buffer);
      }
      break;
    // default: // <- clang thinks this is unnecessary...
    //   g_warning("%s:%d %s: unexpected header %d", __FILE__, __LINE__, __func__, header);
    //   break;
  }

  char *ret = g_strdup(buffer);

  g_free(buffer);
  return ret;
}

static uint8_t *http_extract_content(const char *http)
{
  if (!http)
    return NULL;

  const char *content_start = g_strstr_len(http, -1, "\n\r\n");
  if (!content_start)
    return NULL;

  char  *content_length_str = http_extract_header(http, HTTP_HEADER_CONTENT_LENGTH);
  size_t content_length     = (size_t)toInteger(content_length_str);

  uint8_t *ret = g_malloc(content_length);
  memcpy(ret, content_start, content_length);

  g_free(content_length_str);

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
    struct HttpRequest request;
    // Request line
    request.method = http_extract_request_method(http);
    request.path = http_extract_request_path(http);
    request.version = http_extract_request_version(http);

    // Header fields
    request.host = http_extract_header(http, HTTP_HOST);
    request.content_type = http_extract_header(http, HTTP_CONTENT_TYPE);
    request.content_length = (size_t)toInteger(http_extract_header(http, HTTP_HEADER_CONTENT_LENGTH));
    request.expect = http_extract_header(http, HTTP_HEADER_EXPECT);

    // Content
    request.content = http_extract_content(http);
    return request;
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
    char buffer[8]; // < For store the content-lengt

    // Calculate the buffer size to allocate
    response_size += strlen(http_status_to_string(response.status_code)) + strlen("\n");

    if (response.content_type) {
      response_size += strlen("Content-Type: ") + strlen(response.content_type) + strlen("\n");
    }
    if (response.content_length) {
      // Convert the integer content_length into string
      snprintf(buffer, 8, "%zu", response.content_length);

      response_size += strlen("Content-Length: ") + strlen(buffer) + strlen("\n");
    }
    if (response.connection) {
      response_size += strlen("Connection: ") + strlen(response.connection) + strlen("\n");
    }
    if (response.location) {
      response_size += strlen("Location: ") + strlen(response.location) + strlen("\n");
    }
    if (response.allow) {
      response_size += strlen("Allow: ") + strlen(response.allow);
    }
    response_size += strlen("\n\r\n");

    if (response.content) {
      response_size += response.content_length + strlen("\n");
    }

    char *output = g_malloc(response_size + 1);
    // NOTE: This is required for strcat() to work properly!
    output[0] = '\0';

    // The first line
    strcat(output, http_status_to_string(response.status_code));

    // Header
    if (response.content_type) {
      strcat(output, "\nContent-Type: ");
      strcat(output, response.content_type);
    }
    if (response.content_length) {
      strcat(output, "\nContent-Length: ");
      strcat(output, buffer);
    }
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

    strcat(output, "\n\r\n");

    // Content
    // NOTE: the content may not be ASCII string...
    if (response.content) {
      memcpy(output + strlen(output), response.content, response.content_length);
    }

    return output;
}

/********** Public APIs **********/

char *teapot_http_process(size_t *size, const char *input)
{
    // get the request
    struct HttpRequest request = teapot_http_request_parse(input);
    // All the information sent by client is storing in request now.


    struct HttpResponse response;

    // --- Predefine the connection of the response ---
    response.connection = "close";
    // below variables may be changed later
    response.content_type = NULL;
    response.content_length = 0;
    response.location = NULL;
    response.allow = NULL;
    response.content = NULL;
    // ------------------------------------------------------------

    switch (request.method) {
      case HTTP_GET:
        // Do you want to direct to a new location? ->> 3XX response
        // If the new location is temporart ->> HTTP 302
        if (teapot_redir_302_query(request.path) != NULL) {
          response.status_code = HTTP_STATUS_FOUND;
          response.location = teapot_redir_302_query(request.path);
        }

        struct TeapotFile *file = teapot_file_read("/src/index.html", 0, TEAPOT_FILE_READ_RANGE_FULL);
        if (file == NULL) { // If the file does not exist.
          response.status_code = HTTP_STATUS_NOT_FOUND; ///< HTTP 404
        } else {
          response.status_code = HTTP_STATUS_OK; ///< HTTP 200
          response.content_type = file -> content_type;
          response.content_length = file -> size;
          response.content = file -> content;
        }
        teapot_file_free(file);
        break;
      case HTTP_HEAD:
        response.status_code = HTTP_STATUS_NO_CONTENT; ///< HTTP 204
      case HTTP_POST:
        if (teapot_file_write(request.content, request.content_length, request.path)) {
          // If successfully post the content
        } else {
          // If fail to post the content
        }
        break;
      case HTTP_DELETE:
        // TODO
        break;
      case HTCPCP_BREW:
        response.status_code = HTCPCP_STATUS_I_AM_A_TEAPOT; 
        break;
      case HTTP_METHOD_UNKNOWN:
      // default:
        response.status_code = HTTP_STATUS_METHOD_NOT_ALLOWED;    ///< HTTP 405
        response.allow = "GET HEAD POST DELETE";
        break;
    }

    char *response_str = teapot_http_response_construct(response);
    // char *response_str = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
    *size = strlen(response_str);// < Record the length of the response string
    return response_str;
}
