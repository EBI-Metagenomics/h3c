#ifndef H3CLIENT_ANSWER_H
#define H3CLIENT_ANSWER_H

#include <stddef.h>
#include <stdio.h>

struct answer;
struct lip_file;

struct answer *answer_new(void);
void answer_del(struct answer const *ans);

unsigned char *answer_status_data(struct answer *ans);
size_t answer_status_size(void);
struct hmmd_status const *answer_status_unpack(struct answer *ans);

enum h3c_rc answer_ensure(struct answer *ans, size_t size);
unsigned char *answer_data(struct answer *ans);
enum h3c_rc answer_unpack(struct answer *ans);

enum h3c_rc answer_pack(struct answer const *ans, struct lip_file *);

void answer_print(struct answer const *ans);

#endif
