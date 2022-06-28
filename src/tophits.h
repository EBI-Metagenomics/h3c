#ifndef TOPHITS_H
#define TOPHITS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct h3c_tophits;
struct lip_file;

struct h3c_tophits
{
    uint64_t nhits;
    struct h3c_hit *hits;
    uint64_t nreported;
    uint64_t nincluded;
    bool is_sorted_by_sortkey;
    bool is_sorted_by_seqidx;
};

void tophits_init(struct h3c_tophits *);
enum h3c_rc tophits_setup(struct h3c_tophits *, uint64_t nhits);
void tophits_cleanup(struct h3c_tophits *);
enum h3c_rc tophits_pack(struct h3c_tophits const *, struct lip_file *);
void tophits_print_targets(struct h3c_tophits const *, FILE *file, double Z);
void tophits_print_domains(struct h3c_tophits const *, FILE *file, double Z,
                           double domZ);

#endif
