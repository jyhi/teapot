#ifndef TEAPOT_SERVER_H
#define TEAPOT_SERVER_H

#include <gio/gio.h>
#include <glib.h>

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
