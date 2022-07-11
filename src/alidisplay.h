#ifndef ALIDISPLAY_H
#define ALIDISPLAY_H

#include "compiler.h"
#include <stdint.h>
#include <stdio.h>

struct lip_file;

struct alidisplay
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
    unsigned N;

    char *hmmname;
    char *hmmacc;
    char *hmmdesc;
    unsigned hmmfrom;
    unsigned hmmto;
    unsigned M;

    char *sqname;
    char *sqacc;
    char *sqdesc;
    unsigned sqfrom;
    unsigned sqto;
    unsigned L;
};

STATIC_ASSERT(sizeof(unsigned) >= 4);

enum h3c_rc alidisplay_init(struct alidisplay *);
void alidisplay_cleanup(struct alidisplay *);
enum h3c_rc alidisplay_pack(struct alidisplay const *, struct lip_file *);
enum h3c_rc alidisplay_unpack(struct alidisplay *, struct lip_file *);
void alidisplay_print(struct alidisplay const *, FILE *file);

#endif
