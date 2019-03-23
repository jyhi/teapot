#include <gio/gio.h>
#include <glib.h>
#include "config.h"

// Ports to bind on
static int http_port  = TEAPOT_DEFAULT_HTTP_PORT;
static int https_port = TEAPOT_DEFAULT_HTTPS_PORT;

int teapot_handle_options(GApplication *app, GVariantDict *opts, gpointer data)
{
  (void) data;

  if (g_variant_dict_contains(opts, "version")) {
    puts(TEAPOT_NAME " " TEAPOT_VERSION);
    return 0; // no future action is needed, exit
  }

  if (g_variant_dict_lookup(opts, "http-port", "i", &http_port)) {
    g_debug("HTTP port set to %d", http_port);
  }

  if (g_variant_dict_lookup(opts, "https-port", "i", &https_port)) {
    g_debug("HTTPS port set to %d", https_port);
  }

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
  g_application_add_main_option(app, "http-port", 'p', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, "Port to bind the HTTP service", "port");
  g_application_add_main_option(app, "https-port", 'P', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, "Port to bind the HTTPS service", "port");
  g_application_add_main_option(app, "version", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, "Show version string", NULL);

  // Register handlers to signals to support running of GApplication
  g_signal_connect(app, "handle-local-options", G_CALLBACK(teapot_handle_options), NULL);
  g_signal_connect(app, "activate", G_CALLBACK(teapot_activate), NULL);

  // Nike
  return g_application_run(app, argc, argv);
}
