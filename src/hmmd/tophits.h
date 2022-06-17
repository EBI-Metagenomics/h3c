#ifndef H3CLIENT_TOPHITS_H
#define H3CLIENT_TOPHITS_H

#include "hmmd/hit.h"
#include <stdbool.h>
#include <stdint.h>

struct hmmd_tophits
{
    struct hmmd_hit **hit;
    struct hmmd_hit *unsrt;
    uint64_t Nalloc;
    uint64_t N;
    uint64_t nreported;
    uint64_t nincluded;
    bool is_sorted_by_sortkey;
    bool is_sorted_by_seqidx;
};

struct hmmd_tophits *hmmd_tophits_new(void);
void hmmd_tophits_del(struct hmmd_tophits const *);

#endif
