#ifndef AMSG_H
#define AMSG_H

#include <stddef.h>

struct amsg;
struct nng_stream;

struct amsg *arecv(struct nng_stream *, size_t len, void *data,
                   void (*callb)(void *), void *arg);
int await(struct amsg *);
void adel(struct amsg *);
void acancel(struct amsg *);
int aresult(struct amsg *);

#endif
