#ifndef H3CLIENT_H3CLIENT_H
#define H3CLIENT_H3CLIENT_H

#include "h3client/rc.h"
#include <stdint.h>
#include <stdio.h>

enum h3c_rc h3c_open(char const *ip, uint16_t port);
enum h3c_rc h3c_call(char const *args, FILE *fasta);
enum h3c_rc h3c_pack_answer(FILE *h3answer);
enum h3c_rc h3c_close(void);

void h3c_pack_print(void);

#endif
