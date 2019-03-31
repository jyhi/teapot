#include <stdio.h>
#include <gio/gio.h>
#include <glib.h>
#include "server.h"
#include "app.h"
#include "redir.h"
#include "config.h"

// Address and ports to bind on
static struct TeapotHttpBinding http_binding = {
  TEAPOT_DEFAULT_BIND_ADDRESS,
  TEAPOT_DEFAULT_HTTP_PORT,
};

static struct TeapotHttpsBinding https_binding = {
  TEAPOT_DEFAULT_BIND_ADDRESS,
  TEAPOT_DEFAULT_HTTPS_PORT,
  TEAPOT_DEFAULT_TLS_CERTIFICATE_PATH,
  TEAPOT_DEFAULT_TLS_PRIVATE_KEY_PATH,
};

/********** Private APIs **********/

static int teapot_read_config_file(const char *path)
{
  gchar   *temp_str  = NULL;
  gint32   temp_port = 0;
  GError  *error     = NULL;
  gboolean r         = FALSE;

  GKeyFile *conf = g_key_file_new();

  r = g_key_file_load_from_file(conf, path, G_KEY_FILE_NONE, &error);
  if (!r) {
    g_printerr("Cannot load config file %s: %s\n", path, error->message);
    g_clear_error(&error);
    return 1;
  }

  g_message("Reading configuration file %s", path);

  temp_str = g_key_file_get_string(conf, "Teapot", "bind", &error);
  if (temp_str) {
    http_binding.address  = g_strdup(temp_str);
    https_binding.address = g_strdup(temp_str);
    g_free(temp_str);
  }

  temp_str = g_key_file_get_string(conf, "Teapot", "cert", &error);
  if (temp_str) {
    https_binding.cert_path = g_strdup(temp_str);
    g_free(temp_str);
  }

  temp_str = g_key_file_get_string(conf, "Teapot", "key", &error);
  if (temp_str) {
    https_binding.pkey_path = g_strdup(temp_str);
    g_free(temp_str);
  }

  temp_port = (gint32)g_key_file_get_integer(conf, "Teapot", "http-port", &error);
  if (temp_str != 0) {
    // Port number is actually from 1 to 65535
    if (temp_port < 1 || temp_port > G_MAXUINT16) {
      g_printerr("Port number should range from 1 to 65535.\n");
      return 1;
    }

    http_binding.port = (guint16)CLAMP(temp_port, 1, G_MAXUINT16);
  }

  temp_port = (gint32)g_key_file_get_integer(conf, "Teapot", "https-port", &error);
  if (temp_str != 0) {
    // Port number is actually from 1 to 65535
    if (temp_port < 1 || temp_port > 65535) {
      g_printerr("Port number should range from 1 to 65535.\n");
      return 1;
    }

    https_binding.port = (guint16)CLAMP(temp_port, 1, G_MAXUINT16);
  }

  // Also reads URL section for 302 redirection list
  gsize n_redir_path = 0;
  gchar **redir_path = g_key_file_get_string_list(conf, "URL", "302-path", &n_redir_path, &error);
  if (redir_path) {
    gsize n_redir_target = 0;
    gchar **redir_target = g_key_file_get_string_list(conf, "URL", "302-target", &n_redir_target, &error);
    if (redir_target) {

      g_message("Setting up redirection");

      if (n_redir_path != n_redir_target) {
        // Two lists have different length, not good
        g_warning("Malformed 302 list: number of path and target does not match");

        // Free resources
        g_strfreev(redir_target);
        g_strfreev(redir_path);
        g_key_file_free(conf);

        // In this case we shall not continue
        return 1;
      }

      // Build the hash table (with destroy notifiers)
      GHashTable *redir_list = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

      for (gsize i = 0; i < n_redir_path; i++) {
        g_debug("302: %s -> %s", redir_path[i], redir_target[i]);
        g_hash_table_insert(redir_list, g_strdup(redir_path[i]), g_strdup(redir_target[i]));
      }

      // Put the hash table into our provider
      teapot_redir_302_init(redir_list);

      // The provider holds a reference, so we unref here
      g_hash_table_unref(redir_list);

      // Free unused memory, they are already duplicated into the hash table
      g_strfreev(redir_target);
      g_strfreev(redir_path);

    } else {
      g_warning("Malformed 302 list: %s", error->message);
      g_clear_error(&error);
    }
  } else {} // we just skip it if not defined

  g_key_file_free(conf);

  return 0;
}

static int teapot_handle_options(GApplication *app, GVariantDict *opts, gpointer data)
{
  (void) app;
  (void) data;
  gchar *temp_str  = NULL;
  gint32 temp_port = 0;

  if (g_variant_dict_contains(opts, "version")) {
    puts(TEAPOT_NAME " " TEAPOT_VERSION);
    return 0; // no future action is needed, exit
  }

  // Read configuration file first (so the command line will win)
  if (g_variant_dict_lookup(opts, "config", "s", &temp_str) && temp_str) {

    // The configuration file specified in command line wins
    int r = teapot_read_config_file(temp_str);
    g_free(temp_str);

    // An error occurred during configuration reading
    if (r != 0)
      return r;

  } else {

    // If no configuration file is specified, try reading it from the default path
    GFile *default_config = g_file_new_for_path(TEAPOT_DEFAULT_CONFIG_FILE_PATH);
    int r = 0;

    if (g_file_query_exists(default_config, NULL))
      r = teapot_read_config_file(TEAPOT_DEFAULT_CONFIG_FILE_PATH);

    g_clear_object(&default_config);

    // An error occurred during configuration reading
    if (r != 0)
      return r;

  }

  if (g_variant_dict_lookup(opts, "bind", "s", &temp_str) && temp_str) {
    // If this is specified in the config already, free this unused memory
    g_free(http_binding.address);
    g_free(https_binding.address);

    http_binding.address  = g_strdup(temp_str);
    https_binding.address = g_strdup(temp_str);
    g_free(temp_str);
  }

  if (g_variant_dict_lookup(opts, "cert", "s", &temp_str) && temp_str) {
    g_free(https_binding.cert_path);

    https_binding.cert_path = g_strdup(temp_str);
    g_free(temp_str);
  }

  if (g_variant_dict_lookup(opts, "key", "s", &temp_str) && temp_str) {
    g_free(https_binding.pkey_path);

    https_binding.pkey_path = g_strdup(temp_str);
    g_free(temp_str);
  }

  if (g_variant_dict_lookup(opts, "http-port", "i", &temp_port)) {
    // Port number is actually from 1 to 65535
    if (temp_port < 1 || temp_port > G_MAXUINT16) {
      g_printerr("Port number should range from 1 to 65535.\n");
      return 1;
    }

    http_binding.port = (guint16)CLAMP(temp_port, 1, G_MAXUINT16);
  }

  if (g_variant_dict_lookup(opts, "https-port", "i", &temp_port)) {
    // Port number is actually from 1 to 65535
    if (temp_port < 1 || temp_port > 65535) {
      g_printerr("Port number should range from 1 to 65535.\n");
      return 1;
    }

    https_binding.port = (guint16)CLAMP(temp_port, 1, G_MAXUINT16);
  }

  // Two ports cannot be the same
  if (http_binding.port == https_binding.port) {
    g_printerr("HTTP and HTTPS binding ports cannot be the same.\n");
    return 1;
  }

  // BUG: 0 is not catched by the command line argument parser, so setting a
  // port to 0 will fall directly to the default ones. I don't know why.
  g_debug("HTTP binding set to %s:%d%s", http_binding.address, http_binding.port, http_binding.port == TEAPOT_DEFAULT_HTTP_PORT ? " (default)" : "");
  g_debug("HTTPS binding set to %s:%d%s", https_binding.address, https_binding.port, https_binding.port == TEAPOT_DEFAULT_HTTPS_PORT ? " (default)" : "");
  g_debug("TLS certificate path set to %s", https_binding.cert_path);
  g_debug("TLS peivate key path set to %s", https_binding.pkey_path);

  // A negative exit code let GApplication continue to run
  return -1;
}

static void teapot_activate(GApplication *app, gpointer data)
{
  (void) data;

  // Allocate memory

  g_message("%s %s starting", TEAPOT_NAME, TEAPOT_VERSION);

  // Increase reference count of the application, we are going to ignite
  g_application_hold(app);

  // Spawn HTTP listener
  g_thread_unref(g_thread_new("http_listener", (GThreadFunc)teapot_http_listener, &http_binding));

  // Spawn HTTPS listener
  g_thread_unref(g_thread_new("https_listener", (GThreadFunc)teapot_https_listener, &https_binding));

  // This handler will return, but since we've increased the refcount of app,
  // GApplication will keep running.
}

/********** Public APIs **********/

int teapot_run(int argc, char **argv)
{
  GApplication *app = g_application_new("moe.uic.teapot", G_APPLICATION_FLAGS_NONE);

  // Adds description to the help manual (--help)
  g_application_set_option_context_parameter_string(app, "- a simple HTTP(S) server");
  g_application_add_main_option(app, "config", 'C', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, "Configuration file to use", "path");
  g_application_add_main_option(app, "bind", 'b', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, "Address to bind", "address");
  g_application_add_main_option(app, "http-port", 'p', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, "Port to bind the HTTP service", "port");
  g_application_add_main_option(app, "https-port", 'P', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, "Port to bind the HTTPS service", "port");
  g_application_add_main_option(app, "cert", 'c', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, "The TLS certificate to use", "path");
  g_application_add_main_option(app, "key", 'k', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, "The TLS private key to use", "path");
  g_application_add_main_option(app, "version", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, "Show version string", NULL);

  // Register handlers to signals to support running of GApplication
  g_signal_connect(app, "handle-local-options", G_CALLBACK(teapot_handle_options), NULL);
  g_signal_connect(app, "activate", G_CALLBACK(teapot_activate), NULL);

  // Nike
  int ret = g_application_run(app, argc, argv);

  // Free unused memory
  g_clear_object(&app);

  return ret;
}
