#ifndef TEAPOT_SERVER_H
#define TEAPOT_SERVER_H

#include <gio/gio.h>
#include <glib.h>

/**
 * Handle options passed to Teapot.
 *
 * This function is designed to be used with the "handle-local-option" signal
 * emitted by GApplication.
 *
 * @param app  [in] The GApplication.
 * @param opts [in] Options passed to the program.
 * @param data [in] Optional user data. No use.
 * @return A return value indicating the next step to go. Refer to documentation
 *         of GApplication for details.
 */
int teapot_handle_options(GApplication *app, GVariantDict *opts, gpointer data);

/**
 * Application entry of Teapot for GApplication.
 *
 * This function is designed to be used with the "activate" signal emitted by
 * GApplication.
 *
 * @param app  [in] The GApplication.
 * @param data [in] Optional user data. No use.
 */
void teapot_activate(GApplication *app, gpointer data);

/**
 * Run Teapot.
 *
 * This function is designed to be chained in the main() function with `return`
 * statement:
 *
 * ```c
 * int main(int argc, char **argv) {
 *   return teapot_run(argc, argv);
 * }
 * ```
 *
 * This function will create a GApplication, register signals and options, and
 * run `g_application_run()` on it. Upon return from `g_application_run()`, the
 * return value is returned to the caller of `teapot_run()` too.
 *
 * @param argc [in] Argument count, from `main()`.
 * @param argv [in] Argument content, from `main()`.
 * @return Status code indicating the program's exit state.
 */
int teapot_run(int argc, char **argv);

#endif
