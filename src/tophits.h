#ifndef TOPHITS_H
#define TOPHITS_H

#include "compiler.h"
#include <stdbool.h>
#include <stdint.h>
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

STATIC_ASSERT(sizeof(unsigned) >= sizeof(uint32_t));

void tophits_init(struct tophits *);
enum h3c_rc tophits_setup(struct tophits *, unsigned nhits);
void tophits_cleanup(struct tophits *);

enum h3c_rc tophits_pack(struct tophits const *, struct lip_file *);
enum h3c_rc tophits_unpack(struct tophits *result, struct lip_file *);

void tophits_print_targets(struct tophits const *, FILE *file, double Z);
void tophits_print_domains(struct tophits const *, FILE *file, double Z,
                           double domZ);

void tophits_print_targets_table(char *qname, char *qacc,
                                 struct tophits const *th, FILE *file,
                                 int show_header, double Z);

void tophits_print_domains_table(char *qname, char *qacc,
                                 struct tophits const *th, FILE *file,
                                 int show_header, double Z, double domZ);

#endif
