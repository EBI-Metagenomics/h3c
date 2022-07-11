#ifndef HIT_H
#define HIT_H

#include "compiler.h"
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
    unsigned nregions;
    unsigned nclustered;
    unsigned noverlaps;
    unsigned nenvelopes;

    unsigned flags;
    unsigned nreported;
    unsigned nincluded;
    unsigned best_domain;

    unsigned ndomains;
    struct domain *domains;
};

STATIC_ASSERT(sizeof(unsigned) >= 4);

enum h3c_rc hit_init(struct hit *);
enum h3c_rc hit_setup(struct hit *, unsigned ndomains);
void hit_cleanup(struct hit *);
enum h3c_rc hit_pack(struct hit const *, struct lip_file *);
enum h3c_rc hit_unpack(struct hit *, struct lip_file *);

#endif
