#ifndef TEAPOT_SERVER_H
#define TEAPOT_SERVER_H

/**
 * Binding information packed together.
 */
struct TeapotBinding {
  gchar  *address; ///< Binding address of Teapot
  guint16 port;    ///< Binding port of Teapot
};

/**
 * The Teapot HTTP listener.
 *
 * This function is designed to be used with GThread to spawn (GThreadFunc):
 *
 * ```c
 * gpointer teapot_http_listener(gpointer data);
 * ```
 *
 * @param bindings [in] Binding information packed into struct TeapotBinding.
 * @return Something.
 */
void *teapot_http_listener(const struct TeapotBinding *binding);

/**
 * The Teapot HTTPS listener.
 *
 * This function is designed to be used with GThread to spawn (GThreadFunc):
 *
 * ```c
 * gpointer teapot_https_listener(gpointer data);
 * ```
 *
 * @param bindings [in] Binding information packed into struct TeapotBinding.
 * @return Something.
 */
void *teapot_https_listener(const struct TeapotBinding *binding);

#endif
