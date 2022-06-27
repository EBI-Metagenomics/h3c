#ifndef H3CLIENT_RESULT_H
#define H3CLIENT_RESULT_H

#include <stdbool.h>
#include <stdio.h>

struct h3c_result;

struct h3c_result *h3c_result_new(void);
void h3c_result_del(struct h3c_result const *result);
enum h3c_rc h3c_result_pack(struct h3c_result const *result, FILE *file);

#endif
