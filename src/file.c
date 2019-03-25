#include <gio/gio.h>
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
  GError  *error = NULL;
  gboolean r     = FALSE;

  // For security consideration, we do not allow file access in parent directories
  gchar *abspath = g_canonicalize_filename(path + 1, NULL);
  g_debug("File: canonicalized filename: %s", abspath);
  if (!g_str_has_prefix(abspath, g_get_current_dir())) {
    g_message("File: requested path goes out of scope, reject");
    g_free(abspath);
    return NULL;
  }

  GFile *file = g_file_new_for_path(abspath);

  // TODO: currently we do not support directory listing
  GFileType file_type = g_file_query_file_type(file, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL);
  if (file_type != G_FILE_TYPE_REGULAR) {
    g_message("File: requested path is not a regular file, reject");
    g_clear_object(&file);
    g_free(abspath);
    return NULL;
  }

  // Query file information
  GFileInfo *info = g_file_query_info(file, "standard::*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);
  if (!info) {
    g_warning("Failed to query file info: %s", error->message);
    g_clear_error(&error);
    g_clear_object(&file);
    g_free(abspath);
    return NULL;
  }

  struct TeapotFile *ret = teapot_file_new();

  ret->filename     = g_strdup(g_file_info_get_name(info));
  ret->content_type = g_strdup(g_file_info_get_content_type(info));

  g_debug("File: %s is of type %s", ret->filename, ret->content_type);

  // Read the whole file into memory
  // FIXME
  r = g_file_load_contents(file, NULL, (char **)&(ret->content), &(ret->size), NULL, &error);
  if (!r) {
    g_warning("Failed to load file into memory: %s", error->message);
    g_clear_error(&error);
    teapot_file_free(ret);
    g_clear_object(&info);
    g_clear_object(&file);
    g_free(abspath);
    return NULL;
  }

  g_debug("File: loaded %lu bytes", ret->size);

  // Free unused memory
  g_clear_object(&info);
  g_clear_object(&file);
  g_free(abspath);

  return ret;
}

bool teapot_file_write(const uint8_t *content, const size_t size, const char *path)
{
  // unimplemented
}
