#ifndef HMMD_DOMAIN_H
#define HMMD_DOMAIN_H

#include "hmmd/alidisplay.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
    bool is_reported;
    bool is_included;
    size_t npos;
    float *scores_per_pos;
    struct hmmd_alidisplay ad;
};

void hmmd_domain_init(struct hmmd_domain *);
void hmmd_domain_cleanup(struct hmmd_domain *);

enum h3c_rc hmmd_domain_unpack(struct hmmd_domain *, size_t *read_size,
                               unsigned char const *data);

struct lip_file;

enum h3c_rc hmmd_domain_pack(struct hmmd_domain const *, struct lip_file *);

#endif