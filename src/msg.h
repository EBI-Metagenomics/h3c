#ifndef MSG_H
#define MSG_H

#include <stddef.h>

struct msg;
struct nng_stream;

struct msg *msend(struct nng_stream *, size_t, void const *);
struct msg *mrecv(struct nng_stream *, size_t, void *);
int mwait(struct msg *);

#endif
