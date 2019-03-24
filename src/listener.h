#ifndef TEAPOT_LISTENER_H
#define TEAPOT_LISTENER_H

/**
 * Binding information packed together.
 */
struct TeapotBindings {
  gchar  *address;    ///< Binding address of Teapot
  guint16 http_port;  ///< HTTP binding port of Teapot
  guint16 https_port; ///< HTTPS binding port of Teapot
};

/**
 * The Teapot listener, who binds Teapot on an address and listen to two ports
 * for HTTP and HTTPS service respectively.
 *
 * This function is designed to be used with GThread to spawn (GThreadFunc):
 *
 * ```c
 * gpointer teapot_listener(gpointer data);
 * ```
 *
 * @param bindings [in] Binding information packed into struct TeapotBindings
 * @return Something.
 */
void *teapot_listener(const struct TeapotBindings *bindings);

#endif
