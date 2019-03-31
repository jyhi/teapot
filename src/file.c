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

  g_debug("File: freeing %s of size %zu", file->filename, file->size);

  if (file->filename)
    g_free(file->filename);

  if (file->content_type)
    g_free(file->content_type);

  if (file->content)
    g_free(file->content);

  file->size = 0;
}

struct TeapotFile *teapot_file_read(const char *path, const size_t start, const size_t range)
{
  GError  *error = NULL;

  // For security consideration, we do not allow file access in parent directories
  gchar *abspath = g_canonicalize_filename(path + 1, NULL);
  g_debug("File: canonicalized filename: %s", abspath);
  if (!g_str_has_prefix(abspath, g_get_current_dir())) {
    g_message("File: requested path goes out of scope, reject");
    g_free(abspath);
    return NULL;
  }

  GFile *file = g_file_new_for_path(abspath);
  g_free(abspath);

  // TODO: currently we do not support directory listing
  GFileType file_type = g_file_query_file_type(file, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL);
  if (file_type != G_FILE_TYPE_REGULAR) {
    g_message("File: requested path is not a regular file, reject");
    g_clear_object(&file);

    return NULL;
  }

  // Query file information
  GFileInfo *info = g_file_query_info(file, "standard::*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);
  if (!info) {
    g_warning("Failed to query file info: %s", error->message);
    g_clear_error(&error);
    g_clear_object(&file);

    return NULL;
  }

  struct TeapotFile *ret = teapot_file_new();

  ret->filename     = g_strdup(g_file_info_get_name(info));
  ret->content_type = g_strdup(g_file_info_get_content_type(info));
  ret->start        = start;

  g_debug("File: %s is of type %s", ret->filename, ret->content_type);

  g_clear_object(&info);

  // Read the file into memory based on the designated start byte and range
  GFileInputStream *stream_in = g_file_read(file, NULL, &error);
  if (!stream_in) {
    g_warning("Failed to create input stream: %s", error->message);
    g_clear_error(&error);
    teapot_file_free(ret);
    g_clear_object(&file);

    return NULL;
  }

  // Skip till start
  gssize bytes_skipped = g_input_stream_skip(G_INPUT_STREAM(stream_in), start, NULL, &error);
  if (bytes_skipped < 0) {
    g_warning("Failed to seek file to start: %s", error->message);
    g_clear_error(&error);
    teapot_file_free(ret);
    g_clear_object(&stream_in);
    g_clear_object(&file);

    return NULL;
  }

  // Allocate memory for buffer
  g_debug("File: allocating %zu bytes of memory", range);
  ret->content = g_malloc(range);

  // Read till range
  gssize bytes_read = g_input_stream_read(G_INPUT_STREAM(stream_in), ret->content, range, NULL, &error);
  if (bytes_read < 0) {
    g_warning("Failed to load file into memory: %s", error->message);
    g_clear_error(&error);
    teapot_file_free(ret);
    g_clear_object(&stream_in);
    g_clear_object(&file);

    return NULL;
  }

  ret->size = (size_t)bytes_read;
  g_debug("File: loaded %zu bytes", ret->size);

  // Free unused memory
  g_clear_object(&file);

  return ret;
}

bool teapot_file_write(const uint8_t *content, const size_t size, const char *path)
{
  GError  *error = NULL;
  gboolean r     = FALSE;

  // For security consideration, we do not allow file access in parent directories
  gchar *abspath = g_canonicalize_filename(path + 1, NULL);
  g_debug("File: canonicalized filename: %s", abspath);
  if (!g_str_has_prefix(abspath, g_get_current_dir())) {
    g_message("File: requested path goes out of scope, reject");
    g_free(abspath);
    return false;
  }

  GFile *file = g_file_new_for_path(abspath);

  // If there is already something, do nothing
  if (g_file_query_exists(file, NULL)) {
    g_message("File: resource already exists, reject");
    g_clear_object(&file);
    g_free(abspath);
    return false;
  }

  // Open a file and write to it
  r = g_file_replace_contents(
    file,
    (const char *)content,
    size,
    NULL,
    FALSE,
    G_FILE_CREATE_REPLACE_DESTINATION,
    NULL,
    NULL,
    &error
  );
  if (!r) {
    g_warning("File: failed to write content into file: %s", error->message);
    g_clear_error(&error);
    g_clear_object(&file);
    g_free(abspath);
    return false;
  }

  g_debug("File: written %zu bytes", size);

  // Free unused memory
  g_clear_object(&file);
  g_free(abspath);

  return true;
}
