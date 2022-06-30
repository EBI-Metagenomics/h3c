#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

struct lip_file;

bool expect_key(struct lip_file *f, char const *key);
bool expect_array_size(struct lip_file *f, unsigned size);
bool expect_map_size(struct lip_file *f, unsigned size);
enum h3c_rc read_string(struct lip_file *f, char **str);

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#endif
