#ifndef H3CLIENT_HMMD_H
#define H3CLIENT_HMMD_H

#include "h3client/h3client.h"
#include <stdint.h>

#define HMMD_STATUS_PACK_SIZE (sizeof(uint32_t) + sizeof(uint64_t))

struct hmmd_status
{
    uint32_t status;
    uint64_t msg_size;
};

enum hmmd_zsetby
{
    p7_ZSETBY_NTARGETS = 0,
    p7_ZSETBY_OPTION = 1,
    p7_ZSETBY_FILEINFO = 2
};

struct hmmd_stats
{
    double elapsed;
    double user;
    double sys;

    double Z;
    double domZ;
    enum hmmd_zsetby Z_setby;
    enum hmmd_zsetby domZ_setby;

    uint64_t nmodels;
    uint64_t nseqs;
    uint64_t n_past_msv;
    uint64_t n_past_bias;
    uint64_t n_past_vit;
    uint64_t n_past_fwd;

    uint64_t nhits;
    uint64_t nreported;
    uint64_t nincluded;
    uint64_t *hit_offsets;
};

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

struct hmmd_domain
{
    int64_t ienv, jenv;
    int64_t iali, jali;
    int64_t iorf, jorf;
    float envsc;
    float domcorrection;
    float dombias;
    float oasc;
    float bitscore;
    double lnP;
    int is_reported;
    int is_included;
    float *scores_per_pos;
    struct hmmd_alidisplay *ad;
};

struct hmmd_hit
{
    char *name;
    char *acc;
    char *desc;
    int window_length;
    double sortkey;

    float score;
    float pre_score;
    float sum_score;

    double lnP;
    double pre_lnP;
    double sum_lnP;

    float nexpected;
    int nregions;
    int nclustered;
    int noverlaps;
    int nenvelopes;
    int ndom;

    uint32_t flags;
    int nreported;
    int nincluded;
    int best_domain;

    int64_t seqidx;
    int64_t subseq_start;

    struct hmmd_domain *dcl;
    int64_t offset;
};

void hmmd_status_unpack(struct hmmd_status *status, unsigned char const *data);

void hmmd_stats_init(struct hmmd_stats *stats);
void hmmd_stats_cleanup(struct hmmd_stats *stats);
enum h3c_rc hmmd_stats_unpack(struct hmmd_stats *stats, size_t *read_size,
                              unsigned char const *data);

#endif
