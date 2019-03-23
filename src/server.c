#include <gio/gio.h>
#include <glib.h>
#include "config.h"

// Ports to bind on
static gchar  *address    = TEAPOT_DEFAULT_BIND_ADDRESS;
static guint16 http_port  = TEAPOT_DEFAULT_HTTP_PORT;
static guint16 https_port = TEAPOT_DEFAULT_HTTPS_PORT;

int teapot_handle_options(GApplication *app, GVariantDict *opts, gpointer data)
{
  (void) data;
  gint32 temp_port = 0;

  if (g_variant_dict_contains(opts, "version")) {
    puts(TEAPOT_NAME " " TEAPOT_VERSION);
    return 0; // no future action is needed, exit
  }

  g_variant_dict_lookup(opts, "bind", "s", &address);

  if (g_variant_dict_lookup(opts, "http-port", "i", &temp_port)) {
    // Port number is actually from 1 to 65535
    if (temp_port < 1 || temp_port > G_MAXUINT16) {
      g_printerr("Port number should range from 1 to 65535.\n");
      return 1;
    }

    http_port = CLAMP(temp_port, 1, G_MAXUINT16);
  }

  if (g_variant_dict_lookup(opts, "https-port", "i", &temp_port)) {
    // Port number is actually from 1 to 65535
    if (temp_port < 1 || temp_port > 65535) {
      g_printerr("Port number should range from 1 to 65535.\n");
      return 1;
    }

    https_port = CLAMP(temp_port, 1, G_MAXUINT16);
  }

  // BUG: 0 is not catched by the command line argument parser, so setting a
  // port to 0 will fall directly to the default ones. I don't know why.
  g_debug("Binding address set to %s", address);
  g_debug("HTTP port set to %d%s", http_port, http_port == TEAPOT_DEFAULT_HTTP_PORT ? " (default)" : "");
  g_debug("HTTPS port set to %d%s", https_port, https_port == TEAPOT_DEFAULT_HTTPS_PORT ? " (default)" : "");

  // A negative exit code let GApplication continue to run
  return -1;
}

void teapot_activate(GApplication *app, gpointer data)
{
  (void) data;

  g_message("%s %s starting", TEAPOT_NAME, TEAPOT_VERSION);

  // unimpl
}

int teapot_run(int argc, char **argv)
{
  GApplication *app = g_application_new("moe.uic.teapot", G_APPLICATION_FLAGS_NONE);

  // Adds description to the help manual (--help)
  g_application_set_option_context_parameter_string(app, "- a simple HTTP(S) server");
  g_application_add_main_option(app, "bind", 'b', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, "Address to bind", "address");
  g_application_add_main_option(app, "http-port", 'p', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, "Port to bind the HTTP service", "port");
  g_application_add_main_option(app, "https-port", 'P', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, "Port to bind the HTTPS service", "port");
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
