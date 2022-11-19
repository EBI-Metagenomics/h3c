#ifndef H3C_BUFF_H
#define H3C_BUFF_H

#include <stddef.h>

struct buff
{
    size_t size;
    size_t capacity;
    unsigned char data[];
};

struct buff *buff_new(size_t capacity);
int buff_ensure(struct buff **buff, size_t capacity);
void buff_del(struct buff const *buff);

#endif
