#include "answer.h"
#include "buff.h"
#include "h3client/rc.h"
#include "hmmd/hmmd.h"
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

#if 0
    enum h3c_rc rc =
        hmmd_stats_unpack(&ans->stats, &read_size, ans->buff->data);
    struct hmmd_tophits *th = p7_tophits_Create();

    free(th->unsrt);
    free(th->hit);

    th->N = stats->nhits;
    if ((th->unsrt = malloc(stats->nhits * sizeof(P7_HIT))) == NULL)
    {
        fprintf(stderr, "[%s:%d] malloc error %d - %s\n", __FILE__, __LINE__,
                errno, strerror(errno));
        exit(1);
    }
    th->nreported = stats->nreported;
    th->nincluded = stats->nincluded;
    th->is_sorted_by_seqidx = FALSE;
    th->is_sorted_by_sortkey = TRUE;

    if ((th->hit = malloc(sizeof(void *) * stats->nhits)) == NULL)
    {
        fprintf(stderr, "[%s:%d] malloc error %d - %s\n", __FILE__, __LINE__,
                errno, strerror(errno));
        exit(1);
    }
    hits_start = buf_offset;
    // deserialize the hits
    for (i = 0; i < stats->nhits; ++i)
    {
        // set all internal pointers of the hit to NULL before deserializing
        // into it
        th->unsrt[i].name = NULL;
        th->unsrt[i].acc = NULL;
        th->unsrt[i].desc = NULL;
        th->unsrt[i].dcl = NULL;
        if ((buf_offset - hits_start) != stats->hit_offsets[i])
        {
            printf("Hit offset %d did not match expected.  Found %d, expected "
                   "%" PRIu64 "\n",
                   i, (buf_offset - hits_start), stats->hit_offsets[i]);
        }
        if (p7_hit_Deserialize(buf, &buf_offset, &(th->unsrt[i])) != eslOK)
        {
            printf("Unable to deserialize hit %d\n", i);
            exit(0);
        }
        th->hit[i] = &(th->unsrt[i]);
    }
#endif
}
