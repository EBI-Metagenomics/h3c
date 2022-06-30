#ifndef STATS_H
#define STATS_H

#include "zsetby.h"
#include <stdint.h>

struct stats;
struct lip_file;

struct stats
{
    double Z;
    double domZ;

    enum zsetby Z_setby;
    enum zsetby domZ_setby;

    uint32_t nmodels;
    uint32_t nseqs;
    uint32_t n_past_msv;
    uint32_t n_past_bias;
    uint32_t n_past_vit;
    uint32_t n_past_fwd;

    uint32_t nhits;
    uint32_t nreported;
    uint32_t nincluded;
};

void stats_init(struct stats *);
enum h3c_rc stats_pack(struct stats const *, struct lip_file *);
enum h3c_rc stats_unpack(struct stats *, struct lip_file *);

#endif
