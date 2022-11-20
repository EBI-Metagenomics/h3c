#ifndef HMSG_H
#define HMSG_H

#include <stddef.h>

struct answer;
struct hmsg;
struct nng_stream;

struct hmsg *hrecv(struct nng_stream *, struct answer *, int timeout);
int hwait(struct hmsg *);
void hdel(struct hmsg *);
void hcancel(struct hmsg *);
int hresult(struct hmsg *);
struct answer *houtput(struct hmsg *);

#endif
