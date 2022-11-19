#ifndef MSG_H
#define MSG_H

#include <stddef.h>

struct nng_stream;
struct msg;

struct msg *msend(struct nng_stream *, void *, size_t);
struct msg *mrecv(struct nng_stream *, void *, size_t);
int mwait(struct msg *);

#endif
