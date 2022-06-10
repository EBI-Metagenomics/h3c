#ifndef H3CLIENT_ANSWER_H
#define H3CLIENT_ANSWER_H

#include "hmmd.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct buff;

struct h3answer_status
{
    uint32_t status;
    uint64_t msg_size;
};

struct h3answer
{
    struct
    {
        unsigned char data[HMMD_STATUS_PACK_SIZE];
        struct hmmd_status value;
    } status;

    struct buff *buff;
};

bool h3answer_init(struct h3answer *ans);

unsigned char *h3answer_status_data(struct h3answer *ans);
size_t h3answer_status_size(void);
struct hmmd_status const *h3answer_status_unpack(struct h3answer *ans);

bool h3answer_ensure(struct h3answer *ans, size_t size);
unsigned char *h3answer_data(struct h3answer *ans);
// bool h3answer_parse(struct h3answer *ans);

void h3answer_cleanup(struct h3answer const *ans);

#endif
