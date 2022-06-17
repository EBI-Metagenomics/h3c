#ifndef HMMD_DOMAIN_H
#define HMMD_DOMAIN_H

#include <stddef.h>
#include <stdint.h>

struct hmmd_alidisplay;

struct hmmd_domain
{
    int64_t ienv, jenv;
    int64_t iali, jali;
    int64_t iorf, jorf;
    float envsc;
    float domcorrection;
    float dombias;
    float oasc;
    float bitscore;
    double lnP;
    int is_reported;
    int is_included;
    float *scores_per_pos;
    struct hmmd_alidisplay *ad;
};

enum h3c_rc hmmd_domain_unpack(struct hmmd_domain *dom, size_t *read_size,
                               unsigned char const *data);

#endif
