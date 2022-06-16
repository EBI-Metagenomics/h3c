#include "answer.h"
#include "buff.h"
#include "h3client/h3client.h"
#include "hmmd.h"
#include <stdlib.h>

#define BUFF_SIZE 2048

struct answer
{
    struct
    {
        unsigned char data[HMMD_STATUS_PACK_SIZE];
        struct hmmd_status value;
    } status;

    struct buff *buff;
    struct hmmd_stats stats;
};

struct answer *answer_new(void)
{
    struct answer *ans = malloc(sizeof(*ans));
    if (!ans) return 0;

    ans->status.data[0] = '\0';

    if (!(ans->buff = buff_new(BUFF_SIZE)))
    {
        free(ans);
        return 0;
    }
    hmmd_stats_init(&ans->stats);
    return ans;
}

void answer_del(struct answer const *ans)
{
    buff_del(ans->buff);
    hmmd_stats_cleanup((struct hmmd_stats *)&ans->stats);
    free((void *)ans);
}

unsigned char *answer_status_data(struct answer *ans)
{
    return ans->status.data;
}

size_t answer_status_size(void) { return HMMD_STATUS_PACK_SIZE; }

struct hmmd_status const *answer_status_unpack(struct answer *ans)
{
    hmmd_status_unpack(&ans->status.value, ans->status.data);
    return &ans->status.value;
}

enum h3c_rc answer_ensure(struct answer *ans, size_t size)
{
    return buff_ensure(&ans->buff, size);
}

unsigned char *answer_data(struct answer *ans) { return ans->buff->data; }

enum h3c_rc answer_unpack(struct answer *ans)
{
    size_t read_size = 0;
    return hmmd_stats_unpack(&ans->stats, &read_size, ans->buff->data);
}
