#ifndef HMMD_TOPHITS_H
#define HMMD_TOPHITS_H

#include <stdbool.h>
#include <stdint.h>

struct hmmd_hit;

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

enum h3c_rc hmmd_tophits_init(struct hmmd_tophits *);
void hmmd_tophits_cleanup(struct hmmd_tophits *);

#endif
