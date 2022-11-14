#ifndef H3C_H3C_H
#define H3C_H3C_H

#include "h3c/rc.h"
#include "h3c/result.h"
#include <stdio.h>

int h3c_open(char const *ip, int port, long deadline);
int h3c_close(long deadline);

int h3c_begin(char const *args, long deadline);
int h3c_put(char const *seq, long deadline);
int h3c_end(struct h3c_result *, long deadline);

int h3c_send(char const *args, FILE *fasta, struct h3c_result *, long deadline);

long h3c_now(void);

#endif
