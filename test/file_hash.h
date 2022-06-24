#ifndef TEST_FILE_HASH_H
#define TEST_FILE_HASH_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

bool file_hash(FILE *restrict fp, int64_t *hash);

#endif
