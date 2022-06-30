#ifndef DOMAIN_H
#define DOMAIN_H

#include "alidisplay.h"
#include <stdbool.h>
#include <stdint.h>

struct domain;
struct lip_file;

struct domain
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
    struct alidisplay ad;
};

void domain_init(struct domain *);
enum h3c_rc domain_setup(struct domain *, uint64_t npos);
void domain_cleanup(struct domain *);
enum h3c_rc domain_pack(struct domain const *, struct lip_file *);
enum h3c_rc domain_unpack(struct domain *, struct lip_file *);

#endif
