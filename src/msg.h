#ifndef MSG_H
#define MSG_H

#include <stddef.h>

struct msg;
struct nng_iov;
struct nng_stream;

struct msg *msend(struct nng_stream *, int len, struct nng_iov *);
struct msg *mrecv(struct nng_stream *, int len, struct nng_iov *);
struct msg *mrecv_callb(struct nng_stream *, size_t size, void *data,
                        void (*callb)(void *), void *arg);
int mwait(struct msg *);

#endif
