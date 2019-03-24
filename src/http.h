#ifndef TEAPOT_HTTP_H
#define TEAPOT_HTTP_H

/**
 * Given an HTTP request string, process it, and give a HTTP output.
 *
 * Memory of `output` and `input` are allocated by the caller.
 *
 * Note that this function does not return anything, so it must success (at
 * least something should be given in `output`).
 *
 * @param output [out] The HTTP response produced by the server
 * @param input  [in]  The HTTP request given by the client
 */
void teapot_http_process(char *output, const char *input);

#endif
