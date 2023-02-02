#ifndef H3C_ANSWER_H
#define H3C_ANSWER_H

#include <stddef.h>
#include <stdio.h>

struct answer;
struct h3c_result;

struct answer *h3c_answer_new(void);
void h3c_answer_del(struct answer const *ans);

unsigned char *h3c_answer_status_data(struct answer *ans);
size_t h3c_answer_status_size(void);
struct hmmd_status const *h3c_answer_status_parse(struct answer *ans);
struct hmmd_status const *h3c_answer_status(struct answer const *ans);

int h3c_answer_setup_size(struct answer *ans, size_t size);
unsigned char *h3c_answer_data(struct answer *ans);
int h3c_answer_parse(struct answer *ans);
int h3c_answer_parse_error(struct answer *ans);
int h3c_answer_copy(struct answer *ans, struct h3c_result *);

#endif
