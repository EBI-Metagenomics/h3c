#ifndef AMSG_H
#define AMSG_H

#include <stddef.h>

struct amsg;
struct nng_iov;
struct nng_stream;

struct amsg *h3c_asend(struct nng_stream *, int len, struct nng_iov *,
                       void (*callb)(void *), void *arg, long deadline);
struct amsg *h3c_arecv(struct nng_stream *, size_t len, void *data,
                       void (*callb)(void *), void *arg, long deadline);
void h3c_astart(struct amsg *);
void h3c_adel(struct amsg *);
void h3c_acancel(struct amsg *);
void h3c_astop(struct amsg *);
int h3c_aresult(struct amsg *);

#endif
