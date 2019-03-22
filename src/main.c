#include <glib.h>
#include "util.h"
#include "config.h"

int main(int argc, char **argv)
{
  g_message("%s %s starting", TEAPOT_NAME, TEAPOT_VERSION);

  return 0;
}
