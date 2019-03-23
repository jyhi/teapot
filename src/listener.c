#include <glib.h>
#include "listener.h"

gpointer teapot_listener(gpointer data)
{
  struct TeapotBindings bindings = *(struct TeapotBindings *)data;

  g_debug("In listener: {%s, %d, %d}", bindings.address, bindings.http_port, bindings.https_port);

  return NULL;
}
