#ifndef H3CLIENT_HIT_H
#define H3CLIENT_HIT_H

#include <stddef.h>
#include <stdint.h>

struct hmmd_domain;

struct hmmd_hit
{
    char *name;
    char *acc;
    char *desc;
    int window_length;
    double sortkey;

    float score;
    float pre_score;
    float sum_score;

    double lnP;
    double pre_lnP;
    double sum_lnP;

    float nexpected;
    int nregions;
    int nclustered;
    int noverlaps;
    int nenvelopes;
    int ndom;

    uint32_t flags;
    int nreported;
    int nincluded;
    int best_domain;

    int64_t seqidx;
    int64_t subseq_start;

    struct hmmd_domain *dcl;
    int64_t offset;
};

void hmmd_hit_cleanup(struct hmmd_hit *hit);

enum h3c_rc hmmd_hit_unpack(struct hmmd_hit *hit, size_t *read_size,
                            unsigned char const *data);

#endif
