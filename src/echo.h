#ifndef ECHO_H
#define ECHO_H

#include <stdio.h>

void echo(FILE *stream, char const *fmt, ...)
    __attribute__((format(printf, 2, 3)));

#endif
