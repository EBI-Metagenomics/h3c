#ifndef TOPHITS_H
#define TOPHITS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct tophits;
struct lip_file;

struct tophits
{
    uint64_t nhits;
    struct hit *hits;
    uint64_t nreported;
    uint64_t nincluded;
    bool is_sorted_by_sortkey;
    bool is_sorted_by_seqidx;
};

void tophits_init(struct tophits *);
enum h3c_rc tophits_setup(struct tophits *, uint64_t nhits);
void tophits_cleanup(struct tophits *);
enum h3c_rc tophits_pack(struct tophits const *, struct lip_file *);

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
