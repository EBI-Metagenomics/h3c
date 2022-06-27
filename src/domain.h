#ifndef DOMAIN_H
#define DOMAIN_H

#include "alidisplay.h"
#include <stdbool.h>
#include <stdint.h>

struct h3c_domain;
struct lip_file;

struct h3c_domain
{
    uint64_t ienv;
    uint64_t jenv;
    uint64_t iali;
    uint64_t jali;
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
    struct h3c_alidisplay ad;
};

void domain_init(struct h3c_domain *);
enum h3c_rc domain_setup(struct h3c_domain *, uint64_t npos);
void domain_cleanup(struct h3c_domain *);
enum h3c_rc domain_pack(struct h3c_domain const *, struct lip_file *);

#endif
