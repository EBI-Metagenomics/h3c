#ifndef STATS_H
#define STATS_H

#include "compiler.h"
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

    unsigned nmodels;
    unsigned nseqs;
    unsigned n_past_msv;
    unsigned n_past_bias;
    unsigned n_past_vit;
    unsigned n_past_fwd;

    unsigned nhits;
    unsigned nreported;
    unsigned nincluded;
};

STATIC_ASSERT(sizeof(unsigned) >= 4);

void stats_init(struct stats *);
enum h3c_rc stats_pack(struct stats const *, struct lip_file *);
enum h3c_rc stats_unpack(struct stats *, struct lip_file *);

#endif
