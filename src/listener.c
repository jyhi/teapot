#include <glib.h>
#include "listener.h"

void *teapot_listener(const struct TeapotBindings *bindings)
{
  g_debug("In listener: {%s, %d, %d}", bindings->address, bindings->http_port, bindings->https_port);

  return NULL;
}
