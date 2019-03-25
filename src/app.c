#include <stdio.h>
#include <gio/gio.h>
#include <glib.h>
#include "server.h"
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

  if (g_variant_dict_lookup(opts, "bind", "s", &temp_str) && temp_str) {
    http_binding.address  = g_strdup(temp_str);
    https_binding.address = g_strdup(temp_str);
    g_free(temp_str);
  }

  if (g_variant_dict_lookup(opts, "cert", "s", &temp_str) && temp_str) {
    https_binding.cert_path = g_strdup(temp_str);
    g_free(temp_str);
  }

  if (g_variant_dict_lookup(opts, "key", "s", &temp_str) && temp_str) {
    https_binding.pkey_path = g_strdup(temp_str);
    g_free(temp_str);
  }

  if (g_variant_dict_lookup(opts, "http-port", "i", &temp_port)) {
    // Port number is actually from 1 to 65535
    if (temp_port < 1 || temp_port > G_MAXUINT16) {
      g_printerr("Port number should range from 1 to 65535.\n");
      return 1;
    }

    http_binding.port = CLAMP(temp_port, 1, G_MAXUINT16);
  }

  if (g_variant_dict_lookup(opts, "https-port", "i", &temp_port)) {
    // Port number is actually from 1 to 65535
    if (temp_port < 1 || temp_port > 65535) {
      g_printerr("Port number should range from 1 to 65535.\n");
      return 1;
    }

    https_binding.port = CLAMP(temp_port, 1, G_MAXUINT16);
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
