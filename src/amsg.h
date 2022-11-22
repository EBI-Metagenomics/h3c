#ifndef AMSG_H
#define AMSG_H

#include <stddef.h>

struct amsg;
struct nng_iov;
struct nng_stream;

struct amsg *asend(struct nng_stream *, int len, struct nng_iov *,
                   void (*callb)(void *), void *arg, long deadline);
struct amsg *arecv(struct nng_stream *, size_t len, void *data,
                   void (*callb)(void *), void *arg, long deadline);
void astart(struct amsg *);
void adel(struct amsg *);
void acancel(struct amsg *);
void astop(struct amsg *);
int aresult(struct amsg *);

#endif
