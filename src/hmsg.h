#ifndef HMSG_H
#define HMSG_H

#include <stddef.h>

struct answer;
struct hmsg;
struct nng_stream;

struct hmsg *h3c_hrecv(struct nng_stream *, struct answer *,
                       void (*callb)(void *), void *arg, long deadline);
void h3c_hstart(struct hmsg *);
int h3c_hwait(struct hmsg *);
void h3c_hcancel(struct hmsg *);
void h3c_hstop(struct hmsg *);
void h3c_hdel(struct hmsg *);

int h3c_hresult(struct hmsg *);
struct answer *h3c_houtput(struct hmsg *);

#endif
