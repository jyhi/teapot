#include <glib.h>
#include "server.h"

void *teapot_http_listener(const struct TeapotHttpBinding *binding)
{
  g_debug("In HTTP listener: {%s, %d}", binding->address, binding->port);

  return NULL;
}

void *teapot_https_listener(const struct TeapotHttpsBinding *binding)
{
  g_debug("In HTTPS listener: {%s, %d, %s, %s}", binding->address, binding->port, binding->cert_path, binding->pkey_path);

  return NULL;
}
