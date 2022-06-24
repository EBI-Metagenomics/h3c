#ifndef HMMD_TOPHITS_H
#define HMMD_TOPHITS_H

#include <stdbool.h>
#include <stdint.h>

struct hmmd_hit;
struct lip_file;

struct hmmd_tophits
{
    struct hmmd_hit **hit;
    struct hmmd_hit *unsrt;
    uint64_t nhits;
    uint64_t nreported;
    uint64_t nincluded;
    bool is_sorted_by_sortkey;
    bool is_sorted_by_seqidx;
};

void hmmd_tophits_init(struct hmmd_tophits *);
enum h3c_rc hmmd_tophits_setup(struct hmmd_tophits *, unsigned char const *data,
                               uint64_t nhits, uint64_t nreported,
                               uint64_t nincluded);
void hmmd_tophits_cleanup(struct hmmd_tophits *);

enum h3c_rc hmmd_tophits_pack(struct hmmd_tophits const *, struct lip_file *);

enum h3c_rc hmmd_tophits_print_targets(struct hmmd_tophits const *th,
                                       bool show_accessions, double Z);

int hmmd_tophits_print_domains(struct hmmd_tophits const *th,
                               bool show_accessions, double Z, double domZ,
                               bool show_alignments);

#endif
