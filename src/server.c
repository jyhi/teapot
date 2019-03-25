#include <glib.h>
#include "server.h"

void *teapot_http_listener(const struct TeapotBinding *binding)
{
  g_debug("In HTTP listener: {%s, %d}", binding->address, binding->port);

  return NULL;
}

void *teapot_https_listener(const struct TeapotBinding *binding)
{
  g_debug("In HTTPS listener: {%s, %d}", binding->address, binding->port);

  return NULL;
}
