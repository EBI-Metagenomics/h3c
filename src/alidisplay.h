#ifndef ALIDISPLAY_H
#define ALIDISPLAY_H

#include <stdint.h>
#include <stdio.h>

struct lip_file;

struct h3c_alidisplay
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
};

enum h3c_rc alidisplay_init(struct h3c_alidisplay *);
void alidisplay_cleanup(struct h3c_alidisplay *);
enum h3c_rc alidisplay_pack(struct h3c_alidisplay const *, struct lip_file *);
void alidisplay_print(struct h3c_alidisplay const *, FILE *file);

#endif
