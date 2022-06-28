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

    uint64_t nmodels;
    uint64_t nseqs;
    uint64_t n_past_msv;
    uint64_t n_past_bias;
    uint64_t n_past_vit;
    uint64_t n_past_fwd;

    uint64_t nhits;
    uint64_t nreported;
    uint64_t nincluded;
};

void stats_init(struct stats *);
enum h3c_rc stats_pack(struct stats const *, struct lip_file *);

#endif
