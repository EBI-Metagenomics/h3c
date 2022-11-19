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

void tophits_init(struct tophits *);
int tophits_setup(struct tophits *, unsigned nhits);
void tophits_cleanup(struct tophits *);

int tophits_pack(struct tophits const *, struct lip_file *);
int tophits_unpack(struct tophits *result, struct lip_file *);

void tophits_print_targets(struct tophits const *, FILE *, double Z);
void tophits_print_domains(struct tophits const *, FILE *, double Z,
                           double domZ);

void tophits_print_targets_table(char const *qacc, struct tophits const *th,
                                 FILE *, bool show_header, double Z);

void tophits_print_domains_table(char const *qacc, struct tophits const *th,
                                 FILE *, bool show_header, double Z,
                                 double domZ);

char const *tophits_hit_name(struct tophits const *th, unsigned idx);
char const *tophits_hit_acc(struct tophits const *th, unsigned idx);
double tophits_hit_evalue_ln(struct tophits const *th, unsigned idx, double Z);

#endif
