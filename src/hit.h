#ifndef HIT_H
#define HIT_H

#include <stdint.h>

struct lip_file;
struct h3c_domain;

struct h3c_hit
{
    char *name;
    char *acc;
    char *desc;
    double sortkey;

    float score;
    float pre_score;
    float sum_score;

    double lnP;
    double pre_lnP;
    double sum_lnP;

    float nexpected;
    uint32_t nregions;
    uint32_t nclustered;
    uint32_t noverlaps;
    uint32_t nenvelopes;

    uint32_t flags;
    uint32_t nreported;
    uint32_t nincluded;
    uint32_t best_domain;

    uint32_t ndomains;
    struct h3c_domain *domains;
};

enum h3c_rc hit_init(struct h3c_hit *);
enum h3c_rc hit_setup(struct h3c_hit *, uint32_t ndomains);
void hit_cleanup(struct h3c_hit *);
enum h3c_rc hit_pack(struct h3c_hit const *, struct lip_file *);

#endif
