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
  if (!file)
    return;

  g_debug("File: freeing %s of size %lu", file->filename, file->size);

  if (file->filename)
    g_free(file->filename);

  if (file->content_type)
    g_free(file->content_type);

  if (file->content)
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
