#ifndef HIT_H
#define HIT_H

#include <stdint.h>

struct lip_file;
struct domain;

struct hit
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
    struct domain *domains;
};

enum h3c_rc hit_init(struct hit *);
enum h3c_rc hit_setup(struct hit *, uint32_t ndomains);
void hit_cleanup(struct hit *);
enum h3c_rc hit_pack(struct hit const *, struct lip_file *);

#endif
