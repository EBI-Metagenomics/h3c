#include "h3client/answer.h"
#include "h3client/buff.h"
#include "h3client/hmmd.h"

#define BUFF_SIZE 2048

bool h3answer_init(struct h3answer *ans)
{
    ans->status.data[0] = '\0';
    ans->buff = buff_new(BUFF_SIZE);
    return ans->buff;
}

unsigned char *h3answer_status_data(struct h3answer *ans)
{
    return ans->status.data;
}

size_t h3answer_status_size(void) { return HMMD_STATUS_PACK_SIZE; }

struct hmmd_status const *h3answer_status_unpack(struct h3answer *ans)
{
    hmmd_status_unpack(ans->status.data, &ans->status.value);
    return &ans->status.value;
}

bool h3answer_ensure(struct h3answer *ans, size_t size)
{
    return buff_ensure(&ans->buff, size);
}

unsigned char *h3answer_data(struct h3answer *ans) { return ans->buff->data; }

// bool h3answer_parse(struct h3answer *ans) {}

void h3answer_cleanup(struct h3answer const *ans) { buff_del(ans->buff); }
