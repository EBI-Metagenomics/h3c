#ifndef H3C_H3C_H
#define H3C_H3C_H

#include "h3c/code.h"
#include "h3c/result.h"
#include <stdio.h>

int h3c_open(char const *ip, int port, long deadline);
void h3c_close(void);

int h3c_send(char const *args, char const *seq, long deadline);
void h3c_wait(void);
int h3c_pop(struct h3c_result *r);

long h3c_now(void);

#endif
