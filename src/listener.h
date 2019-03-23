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

gpointer teapot_listener(gpointer data);

#endif
