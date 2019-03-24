#ifndef TEAPOT_FILE_H
#define TEAPOT_FILE_H

// C99 boolean
#ifndef __cplusplus
#include <stdbool.h>
#endif

#include <stdint.h>

/**
 * Read a file from the disk.
 *
 * @param output [out] The NUL-terminated content of the specified file.
 * @param filename [in] Name of the file to read.
 * @return true on success, false on failure.
 */
bool teapot_file_read(uint8_t *output, const char *filename);

/**
 * Write a file from the disk.
 *
 * @param filename [in] Name of the file to read.
 * @param input    [in] The content of the specified file.
 * @param size     [in] The size of the content.
 * @return true on success, false on failure.
 */
bool teapot_file_write(const char *filename, uint8_t *input, size_t size);

#endif
