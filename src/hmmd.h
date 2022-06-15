#ifndef H3CLIENT_HMMD_H
#define H3CLIENT_HMMD_H

#include <stdbool.h>
#include <stdint.h>

#define HMMD_STATUS_PACK_SIZE (sizeof(uint32_t) + sizeof(uint64_t))

struct hmmd_status
{
    uint32_t status;
    uint64_t msg_size;
};

enum hmmd_zsetby
{
    p7_ZSETBY_NTARGETS = 0,
    p7_ZSETBY_OPTION = 1,
    p7_ZSETBY_FILEINFO = 2
};

struct hmmd_stats
{
    double elapsed;
    double user;
    double sys;

    double Z;
    double domZ;
    enum hmmd_zsetby Z_setby;
    enum hmmd_zsetby domZ_setby;

    uint64_t nmodels;
    uint64_t nseqs;
    uint64_t n_past_msv;
    uint64_t n_past_bias;
    uint64_t n_past_vit;
    uint64_t n_past_fwd;

    uint64_t nhits;
    uint64_t nreported;
    uint64_t nincluded;
    uint64_t *hit_offsets;
};

void hmmd_status_unpack(struct hmmd_status *status, unsigned char const *data);

void hmmd_stats_init(struct hmmd_stats *stats);
void hmmd_stats_cleanup(struct hmmd_stats *stats);
bool hmmd_stats_unpack(struct hmmd_stats *stats, unsigned char const *data);

#endif
