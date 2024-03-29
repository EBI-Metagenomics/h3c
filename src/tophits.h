#ifndef TOPHITS_H
#define TOPHITS_H

#include "compiler.h"
#include <stdbool.h>
#include <stdio.h>

struct tophits;
struct lip_file;

struct tophits
{
    unsigned nhits;
    struct hit *hits;
    unsigned nreported;
    unsigned nincluded;
    bool is_sorted_by_sortkey;
    bool is_sorted_by_seqidx;
};

STATIC_ASSERT(sizeof(unsigned) >= 4);

void h3c_tophits_init(struct tophits *);
int h3c_tophits_setup(struct tophits *, unsigned nhits);
void h3c_tophits_cleanup(struct tophits *);

int h3c_tophits_pack(struct tophits const *, struct lip_file *);
int h3c_tophits_unpack(struct tophits *result, struct lip_file *);

void h3c_tophits_print_targets(struct tophits const *, FILE *, double Z);
void h3c_tophits_print_domains(struct tophits const *, FILE *, double Z,
                               double domZ);

void h3c_tophits_print_targets_table(char const *qacc, struct tophits const *th,
                                     FILE *, bool show_header, double Z);

void h3c_tophits_print_domains_table(char const *qacc, struct tophits const *th,
                                     FILE *, bool show_header, double Z,
                                     double domZ);

char const *h3c_tophits_hit_name(struct tophits const *th, unsigned idx);
char const *h3c_tophits_hit_acc(struct tophits const *th, unsigned idx);
double h3c_tophits_hit_evalue_ln(struct tophits const *th, unsigned idx,
                                 double Z);

#endif
