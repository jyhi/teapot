#include <gio/gio.h>
#include "http.h"
#include "server.h"
#include "config.h"

#define BUFSIZE 16384

/********** Private APIs **********/

static void teapot_http_accepter(GSocketConnection *conn)
{
  GError *error = NULL;

  // Get information about the client
  GSocketAddress *remote_addr = g_socket_connection_get_remote_address(conn, &error);
  if (!remote_addr) {
    g_warning("HTTP: failed to retrieve remote address: %s", error->message);
    g_clear_error(&error);

    g_message("HTTP: closing socket");
    g_io_stream_close(G_IO_STREAM(conn), NULL, NULL);
    return;
  }

  gchar  *client_addr = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(remote_addr)));
  guint16 client_port = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(remote_addr));

  g_message("HTTP: accepting connection from %s:%" G_GUINT16_FORMAT, client_addr, client_port);

  // We no longer need the information above (they are only used to show to people)
  g_free(client_addr);
  g_clear_object(&remote_addr);

  // I/O streams
  // NOTE: transfer-none, do not free
  GInputStream  *socket_in  = g_io_stream_get_input_stream(G_IO_STREAM(conn));
  GOutputStream *socket_out = g_io_stream_get_output_stream(G_IO_STREAM(conn));

  gchar *buf_in = g_malloc(BUFSIZE);
  gssize bytes  = 0;

  // Read request into memory
  // FIXME: fixed size buffer
  bytes = g_input_stream_read(socket_in, buf_in, BUFSIZE, NULL, &error);
  if (bytes < 0) {
    g_warning("HTTP: failed to read from the client: %s", error->message);
    g_clear_error(&error);
    g_free(buf_in);

    g_message("HTTP: closing socket");
    g_io_stream_close(G_IO_STREAM(conn), NULL, NULL);
    return;
  }

  g_message("HTTP: read %" G_GSSIZE_FORMAT " bytes", bytes);

  // Handle it
  size_t response_length = 0;
  gchar *buf_out = teapot_http_process(&response_length, buf_in);
  if (!buf_out) {
    g_warning("HTTP: handler failed to process request");
    g_free(buf_in);

    g_message("HTTP: closing socket");
    g_io_stream_close(G_IO_STREAM(conn), NULL, NULL);
    return;
  }

  // Write it back
  bytes = g_output_stream_write(socket_out, buf_out, response_length, NULL, &error);
  if (bytes < 0) {
    g_warning("HTTP: failed to write to the client: %s", error->message);
    g_clear_error(&error);
    g_free(buf_out);
    g_free(buf_in);

    g_message("HTTP: closing socket");
    g_io_stream_close(G_IO_STREAM(conn), NULL, NULL);
    return;
  }

  g_message("HTTP: written %" G_GSSIZE_FORMAT " bytes", bytes);

  // Free resources
  g_free(buf_out);
  g_free(buf_in);

  g_message("HTTP: closing socket");
  g_io_stream_close(G_IO_STREAM(conn), NULL, NULL);
}

static void teapot_https_accepter(GSocketConnection *conn, GTlsCertificate *tls)
{
  GError *error = NULL;

  // Get information about the client
  GSocketAddress *remote_addr = g_socket_connection_get_remote_address(conn, &error);
  if (!remote_addr) {
    g_warning("HTTPS: failed to retrieve remote address: %s", error->message);
    g_clear_error(&error);

    g_message("HTTPS: closing socket");
    g_io_stream_close(G_IO_STREAM(conn), NULL, NULL);
    return;
  }

  gchar  *client_addr = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(remote_addr)));
  guint16 client_port = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(remote_addr));

  g_message("HTTPS: accepting connection from %s:%" G_GUINT16_FORMAT, client_addr, client_port);

  // We no longer need the information above (they are only used to show to people)
  g_free(client_addr);
  g_clear_object(&remote_addr);

  // Wrap the connection with GTlsServerConnection
  GTlsServerConnection *conn_tls = G_TLS_SERVER_CONNECTION(
    g_tls_server_connection_new(G_IO_STREAM(conn), tls, &error)
  );
  if (!conn_tls) {
    g_warning("HTTPS: failed to wrap the stream into a TLS one: %s", error->message);
    g_clear_error(&error);

    g_message("HTTPS: closing socket");
    g_io_stream_close(G_IO_STREAM(conn), NULL, NULL);
    return;
  }

  // I/O streams
  // NOTE: transfer-none, do not free
  GInputStream  *socket_in  = g_io_stream_get_input_stream(G_IO_STREAM(conn_tls));
  GOutputStream *socket_out = g_io_stream_get_output_stream(G_IO_STREAM(conn_tls));

  gchar *buf_in = g_malloc(BUFSIZE);
  gssize bytes  = 0;

  // Read request into memory
  // FIXME: fixed size buffer
  bytes = g_input_stream_read(socket_in, buf_in, BUFSIZE, NULL, &error);
  if (bytes < 0) {
    g_warning("HTTPS: failed to read from the client: %s", error->message);
    g_clear_error(&error);
    g_free(buf_in);

    g_message("HTTPS: closing socket");
    g_io_stream_close(G_IO_STREAM(conn_tls), NULL, NULL);
    g_io_stream_close(G_IO_STREAM(conn), NULL, NULL);
    return;
  }

  g_message("HTTPS: read %" G_GSSIZE_FORMAT " bytes", bytes);

  // Handle it
  size_t response_length = 0;
  gchar *buf_out = teapot_http_process(&response_length, buf_in);
  if (!buf_out) {
    g_warning("HTTPS: handler failed to process request");
    g_free(buf_in);

    g_message("HTTPS: closing socket");
    g_io_stream_close(G_IO_STREAM(conn_tls), NULL, NULL);
    g_io_stream_close(G_IO_STREAM(conn), NULL, NULL);
    return;
  }

  // Write it back
  bytes = g_output_stream_write(socket_out, buf_out, response_length, NULL, &error);
  if (bytes < 0) {
    g_warning("HTTPS: failed to write to the client: %s", error->message);
    g_clear_error(&error);
    g_free(buf_out);
    g_free(buf_in);

    g_message("HTTPS: closing socket");
    g_io_stream_close(G_IO_STREAM(conn_tls), NULL, NULL);
    g_io_stream_close(G_IO_STREAM(conn), NULL, NULL);
    return;
  }

  g_message("HTTPS: written %" G_GSSIZE_FORMAT " bytes", bytes);

  // Free resources
  g_free(buf_out);
  g_free(buf_in);

  g_message("HTTPS: closing socket");
  g_io_stream_close(G_IO_STREAM(conn_tls), NULL, NULL);
  g_io_stream_close(G_IO_STREAM(conn), NULL, NULL);
}

/********** Public APIs **********/

void *teapot_http_listener(const struct TeapotHttpBinding *binding)
{
  g_debug("In HTTP listener: {%s, %d}", binding->address, binding->port);

  GError  *error = NULL;
  gboolean r     = FALSE;

  g_debug("HTTP: creating thread pool");
  GThreadPool *pool = g_thread_pool_new((GFunc *)teapot_http_accepter, NULL, TEAPOT_DEFAULT_THREAD_POOL_MAX_THREADS, FALSE, &error);
  if (error) {
    // "An error can only occur when exclusive is set to TRUE and not all
    // max_threads threads could be created... Note, even in case of error a
    // valid GThreadPool is returned."
    g_message("HTTP: error on creating the thread pool: %s", error->message);
    g_message("HTTP: continue running since pool is valid");
    g_clear_error(&error);
  }

  g_debug("HTTP: creating socket");
  GSocketListener *listener = g_socket_listener_new();
  GSocketAddress  *address  = g_inet_socket_address_new_from_string(binding->address, binding->port);
  GSocketAddress  *effective_address = NULL;

  r = g_socket_listener_add_address(
    listener,
    address,
    G_SOCKET_TYPE_STREAM,
    G_SOCKET_PROTOCOL_TCP,
    NULL,
    &effective_address,
    &error
  );
  if (!r) {
    g_warning("HTTP: failed to create a socket listener: %s", error->message);
    g_clear_error(&error);
    g_clear_object(&address);
    g_clear_object(&listener);

    g_warning("HTTP: can do nothing, exit");
    return NULL;
  }

  gchar *effective_address_str = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(effective_address)));
  g_message("HTTP: service listening on %s:%" G_GUINT16_FORMAT, effective_address_str, g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(effective_address)));
  g_free(effective_address_str);

  for (;;) {
    // Wait for an incoming connection
    GSocketConnection *conn = g_socket_listener_accept(listener, NULL, NULL, &error);
    if (!conn) {
      g_warning("HTTP: failed to accept an incoming socket connection: %s", error->message);
      g_clear_error(&error);
      continue;
    }

    // Spawn an accepter in the thread pool
    r = g_thread_pool_push(pool, conn, &error);
    if (!r) {
      // "An error can only occur when a new thread couldn't be created. In that
      // case data is simply appended to the queue of work to do."
      g_message("HTTP: thread pool throws an error: %s", error->message);
      g_message("HTTP: request is delayed");
      g_clear_error(&error);
    }
  }

  return NULL;
}

void *teapot_https_listener(const struct TeapotHttpsBinding *binding)
{
  g_debug("In HTTPS listener: {%s, %d, %s, %s}", binding->address, binding->port, binding->cert_path, binding->pkey_path);

  GError  *error = NULL;
  gboolean r     = FALSE;

  g_debug("HTTPS: loading certificate");
  GTlsCertificate *tls = g_tls_certificate_new_from_files(binding->cert_path, binding->pkey_path, &error);
  if (!tls) {
    g_warning("HTTPS: failed to load certificate or key file: %s", error->message);
    g_clear_error(&error);

    g_warning("HTTPS: can do nothing, exit");
    return NULL;
  }

  g_debug("HTTPS: creating thread pool");
  GThreadPool *pool = g_thread_pool_new((GFunc *)teapot_http_accepter, tls, TEAPOT_DEFAULT_THREAD_POOL_MAX_THREADS, FALSE, &error);
  if (error) {
    // "An error can only occur when exclusive is set to TRUE and not all
    // max_threads threads could be created... Note, even in case of error a
    // valid GThreadPool is returned."
    g_message("HTTP: error on creating the thread pool: %s", error->message);
    g_message("HTTP: continue running since pool is valid");
    g_clear_error(&error);
  }

  g_debug("HTTPS: creating socket");
  GSocketListener *listener = g_socket_listener_new();
  GSocketAddress  *address  = g_inet_socket_address_new_from_string(binding->address, binding->port);
  GSocketAddress  *effective_address = NULL;

  r = g_socket_listener_add_address(
    listener,
    address,
    G_SOCKET_TYPE_STREAM,
    G_SOCKET_PROTOCOL_TCP,
    NULL,
    &effective_address,
    &error
  );
  if (!r) {
    g_warning("HTTPS: failed to create a socket listener: %s", error->message);
    g_clear_error(&error);
    g_clear_object(&address);
    g_clear_object(&listener);

    g_warning("HTTPS: can do nothing, exit");
    return NULL;
  }

  gchar *effective_address_str = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(effective_address)));
  g_message("HTTPS: service listening on %s:%" G_GUINT16_FORMAT, effective_address_str, g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(effective_address)));
  g_free(effective_address_str);

  for (;;) {
    // Wait for an incoming connection
    GSocketConnection *conn = g_socket_listener_accept(listener, NULL, NULL, &error);
    if (!conn) {
      g_warning("HTTPS: failed to accept an incoming socket connection: %s", error->message);
      g_clear_error(&error);
      continue;
    }

    // Spawn an accepter in the thread pool
    r = g_thread_pool_push(pool, conn, &error);
    if (!r) {
      // "An error can only occur when a new thread couldn't be created. In that
      // case data is simply appended to the queue of work to do."
      g_message("HTTPS: thread pool throws an error: %s", error->message);
      g_message("HTTPS: request is delayed");
      g_clear_error(&error);
    }
  }

  return NULL;
}
