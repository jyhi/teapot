#ifndef TEAPOT_HTTP_H
#define TEAPOT_HTTP_H

/**
 * Given an HTTP request string, process it, and give a HTTP output.
 *
 * @param input [in] The HTTP request given by the client
 * @return The HTTP response produced by the server, NULL on error
 */
char *teapot_http_process(const char *input);

#endif
