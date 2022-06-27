#ifndef HMMD_DOMAIN_H
#define HMMD_DOMAIN_H

#include "hmmd/alidisplay.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct hmmd_domain
{
    uint64_t ienv;
    uint64_t jenv;
    uint64_t iali;
    uint64_t jali;
    // The following two members seems to be receiving
    // random numbers from daemon end. Skip them.
    // int64_t iorf, jorf;
    float envsc;
    float domcorrection;
    float dombias;
    float oasc;
    float bitscore;
    double lnP;
    bool is_reported;
    bool is_included;
    uint64_t npos;
    float *scores_per_pos;
    struct hmmd_alidisplay ad;
};

void hmmd_domain_init(struct hmmd_domain *);
void hmmd_domain_cleanup(struct hmmd_domain *);

enum h3c_rc hmmd_domain_parse(struct hmmd_domain *, size_t *read_size,
                              unsigned char const *data);

#endif
