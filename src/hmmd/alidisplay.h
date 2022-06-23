#ifndef HMMD_ALIDISPLAY_H
#define HMMD_ALIDISPLAY_H

#include <stddef.h>
#include <stdint.h>

struct hmmd_alidisplay
{
    uint8_t presence;

    char *rfline;
    char *mmline;
    char *csline;
    char *model;
    char *mline;
    char *aseq;
    char *ntseq;
    char *ppline;
    uint32_t N;

    char *hmmname;
    char *hmmacc;
    char *hmmdesc;
    uint32_t hmmfrom;
    uint32_t hmmto;
    uint32_t M;

    char *sqname;
    char *sqacc;
    char *sqdesc;
    uint64_t sqfrom;
    uint64_t sqto;
    uint64_t L;

    size_t memsize;
    char *mem;
};

void hmmd_alidisplay_init(struct hmmd_alidisplay *);
void hmmd_alidisplay_cleanup(struct hmmd_alidisplay *);

enum h3c_rc hmmd_alidisplay_unpack(struct hmmd_alidisplay *, size_t *read_size,
                                   unsigned char const *data);

struct lip_file;

enum h3c_rc hmmd_alidisplay_pack(struct hmmd_alidisplay const *,
                                 struct lip_file *);

#endif
