#ifndef DOMAIN_H
#define DOMAIN_H

#include "alidisplay.h"
#include "compiler.h"
#include <stdbool.h>
#include <stdint.h>

struct domain;
struct lip_file;

struct domain
{
    unsigned long ienv;
    unsigned long jenv;
    unsigned long iali;
    unsigned long jali;
    float envsc;
    float domcorrection;
    float dombias;
    float oasc;
    float bitscore;
    double lnP;
    bool is_reported;
    bool is_included;
    unsigned long npos;
    float *scores_per_pos;
    struct alidisplay ad;
};

STATIC_ASSERT(sizeof(long) >= 8);

void domain_init(struct domain *);
enum h3c_rc domain_setup(struct domain *, uint64_t npos);
void domain_cleanup(struct domain *);
enum h3c_rc domain_pack(struct domain const *, struct lip_file *);
enum h3c_rc domain_unpack(struct domain *, struct lip_file *);

#endif
