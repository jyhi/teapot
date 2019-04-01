#include <glib.h>
#include "redir.h"

/********** Internal States **********/

static GHashTable *redir_301_list = NULL;
static GHashTable *redir_302_list = NULL;

/********** Public APIs **********/

void teapot_redir_301_init(GHashTable *table)
{
  if (!redir_301_list) {
    g_debug("Redirection 301 list with %u elements initialized", g_hash_table_size(redir_301_list));
    redir_301_list = g_hash_table_ref(table);
  } else {
    g_warning("Redir: 301: double initialization");
  }
}

void teapot_redir_302_init(GHashTable *table)
{
  if (!redir_302_list) {
    g_debug("Redirection 302 list with %u elements initialized", g_hash_table_size(table));
    redir_302_list = g_hash_table_ref(table);
  } else {
    g_warning("Redir: 302: double initialization");
  }
}

void teapot_redir_301_free(void)
{
  if (redir_301_list) {
    g_debug("Redir: 301: freeing redirection list");
    g_hash_table_destroy(redir_301_list);
    redir_301_list = NULL;
  } else {
    g_warning("Redir: 301: double free");
  }
}

void teapot_redir_302_free(void)
{
  if (redir_302_list) {
    g_debug("Redir: 302: freeing redirection list");
    g_hash_table_destroy(redir_302_list);
    redir_302_list = NULL;
  } else {
    g_warning("Redir: 302: double free");
  }
}

char *teapot_redir_301_query(const char *path)
{
  char *ret = NULL;

  if (redir_301_list)
    ret = g_strdup(g_hash_table_lookup(redir_301_list, path));

  return ret;
}

char *teapot_redir_302_query(const char *path)
{
  char *ret = NULL;

  if (redir_302_list)
    ret = g_strdup(g_hash_table_lookup(redir_302_list, path));

  return ret;
}
