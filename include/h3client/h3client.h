#ifndef H3CLIENT_H3CLIENT_H
#define H3CLIENT_H3CLIENT_H

#include "h3client/rc.h"
#include "h3client/result.h"
#include <stdint.h>
#include <stdio.h>

int h3c_open(char const *ip, uint16_t port);
int h3c_begin(char const *args);
int h3c_send(char const *seq);
int h3c_end(struct h3c_result *);
int h3c_close(void);

int h3c_callf(char const *args, FILE *fasta, struct h3c_result *);

#endif
