#ifndef H3C_BUFF_H
#define H3C_BUFF_H

#include <stddef.h>

struct buff
{
    size_t size;
    size_t capacity;
    unsigned char data[];
};

struct buff *h3c_buff_new(size_t capacity);
int h3c_buff_ensure(struct buff **buff, size_t capacity);
void h3c_buff_del(struct buff const *buff);

#endif
