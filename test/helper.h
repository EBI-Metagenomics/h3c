#ifndef HELPER_H
#define HELPER_H

#include "h3c/h3c.h"
#include <stdio.h>
#include <stdlib.h>

void __check_code(int code, char const *file, int line);
_Noreturn void __fail(char const *file, int line);

#define check_code(code) __check_code(code, __FILE__, __LINE__)
#define fail() __fail(__FILE__, __LINE__)

bool is_close(double a, double b);

#define array_size(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif
