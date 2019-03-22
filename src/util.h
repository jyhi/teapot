#ifndef TEAPOT_UTIL_H
#define TEAPOT_UTIL_H

/**
 * Add position information to debugging outputs
 */
#define debug(format, ...) g_debug("%s: %d: %s(): " format, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#endif
