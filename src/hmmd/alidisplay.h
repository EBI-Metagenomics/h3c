#ifndef HMMD_ALIDISPLAY_H
#define HMMD_ALIDISPLAY_H

#include <stddef.h>
#include <stdint.h>

struct hmmd_alidisplay
{
    char *rfline;
    char *mmline;
    char *csline;
    char *model;
    char *mline;
    char *aseq;
    char *ntseq;
    char *ppline;
    int N;

    char *hmmname;
    char *hmmacc;
    char *hmmdesc;
    int hmmfrom;
    int hmmto;
    int M;

    char *sqname;
    char *sqacc;
    char *sqdesc;
    int64_t sqfrom;
    int64_t sqto;
    int64_t L;

    int memsize;
    char *mem;
};

enum h3c_rc hmmd_alidisplay_unpack(struct hmmd_alidisplay *ali,
                                   size_t *read_size,
                                   unsigned char const *data);

#endif
