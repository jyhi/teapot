#ifndef TEAPOT_REDIR_H
#define TEAPOT_REDIR_H

#include <glib.h>

/**
 * Initialize the 301 redirection list using the given GHashTable.
 *
 * @param table [in] The hash table to initialize. This redirection provider
 *                   holds a reference to the hash table, so you may unref it,
 *                   but do not free it.
 */
void teapot_redir_301_init(GHashTable *table);

/**
 * Initialize the 302 redirection list using the given GHashTable.
 *
 * @param table [in] The hash table to initialize. This redirection provider
 *                   holds a reference to the hash table, so you may unref it,
 *                   but do not free it.
 */
void teapot_redir_302_init(GHashTable *table);

/**
 * Free the 301 redirection list.
 */
void teapot_redir_301_free(void);

/**
 * Free the 301 redirection list.
 */
void teapot_redir_302_free(void);

/**
 * Query whether a given path should be redirected.
 *
 * @param path [in] The path to query.
 * @return NULL if there is no redirection, or a string representing the
 *         location URL. Do not free.
 */
const char *teapot_redir_301_query(const char *path);

/**
 * Query whether a given path should be redirected.
 *
 * @param path [in] The path to query.
 * @return NULL if there is no redirection, or a string representing the
 *         location URL. Do not free.
 */
const char *teapot_redir_302_query(const char *path);

#endif
