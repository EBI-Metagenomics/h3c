#include "answer.h"
#include "buff.h"
#include "h3client/rc.h"
#include "hmmd/hmmd.h"
#include "lite_pack/lite_pack.h"
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
    struct hmmd_tophits tophits;
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
    hmmd_tophits_init(&ans->tophits);
    return ans;
}

void answer_del(struct answer const *ans)
{
    buff_del(ans->buff);
    hmmd_stats_cleanup((struct hmmd_stats *)&ans->stats);
    hmmd_tophits_cleanup((struct hmmd_tophits *)&ans->tophits);
    free((void *)ans);
}

unsigned char *answer_status_data(struct answer *ans)
{
    return ans->status.data;
}

size_t answer_status_size(void) { return HMMD_STATUS_PACK_SIZE; }

struct hmmd_status const *answer_status_unpack(struct answer *ans)
{
    size_t size = 0;
    hmmd_status_deserialize(&ans->status.value, &size, ans->status.data);
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
    enum h3c_rc rc = H3C_OK;
    if ((rc = hmmd_stats_deserialize(&ans->stats, &read_size, ans->buff->data)))
        goto cleanup;

    rc = hmmd_tophits_setup(&ans->tophits, ans->buff->data + read_size,
                            ans->stats.nhits, ans->stats.nreported,
                            ans->stats.nincluded);
    return H3C_OK;

cleanup:
    return rc;
}

enum h3c_rc answer_pack(struct answer const *ans, struct lip_file *f)
{
    enum h3c_rc rc = H3C_OK;

    lip_write_map_size(f, 1);
    lip_write_cstr(f, "h3result");

    lip_write_map_size(f, 2);
    lip_write_cstr(f, "stats");
    if ((rc = hmmd_stats_pack(&ans->stats, f))) return rc;

    lip_write_cstr(f, "tophits");
    return hmmd_tophits_pack(&ans->tophits, f);
}

void answer_print(struct answer const *ans)
{
    hmmd_tophits_print(&ans->tophits, true, ans->stats.Z);
}
