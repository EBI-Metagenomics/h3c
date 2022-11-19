#ifndef SOCK_H
#define SOCK_H

#include <stddef.h>

struct nng_aio;
struct nng_stream;

struct sock
{
    struct nng_aio *aio;
    struct nng_stream *stream;
};

void sock_init(struct sock *);
int sock_open(struct sock *);
void sock_set_stream(struct sock *, struct nng_stream *);
void sock_set_deadline(struct sock *, long deadline);
int sock_send(struct sock *, size_t len, void const *buf);
int sock_recv(struct sock *, size_t len, void *buf);
void sock_close(struct sock *);

#endif
