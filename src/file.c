#include <glib.h>
#include "file.h"

/********** Private APIs **********/

static struct TeapotFile *teapot_file_new(void)
{
  return g_new0(struct TeapotFile, 1);
}

/********** Public APIs **********/

void teapot_file_free(struct TeapotFile *file)
{
  g_free(file->filename);
  g_free(file->content_type);
  g_free(file->content);
  file->size = 0;
}

struct TeapotFile *teapot_file_read(const char *path)
{
  // unimplemented
}

bool teapot_file_write(const uint8_t *content, const size_t size, const char *path)
{
  // unimplemented
}
