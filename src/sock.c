#include "sock.h"
#include "answer.h"
#include "h3c/code.h"
#include "msg.h"
#include "nnge.h"
#include "request.h"
#include "timeout.h"
#include <nng/nng.h>
#include <stddef.h>
#include <stdlib.h>

struct sock
{
    struct nng_aio *aio;
    struct nng_stream *stream;
    struct msg *last_send;
    struct msg *last_recv;
};

struct sock *sock_new(void)
{
    struct sock *sock = malloc(sizeof(*sock));
    if (!sock) return NULL;

    if (nng_aio_alloc(&sock->aio, NULL, NULL))
    {
        free(sock);
        return NULL;
    }

    sock->stream = NULL;
    sock->last_send = NULL;
    sock->last_recv = NULL;

    return sock;
}

void sock_open(struct sock *sock, struct nng_stream *stream)
{
    sock->stream = stream;
}

void sock_set_deadline(struct sock *s, long deadline)
{
    nng_aio_set_timeout(s->aio, timeout(deadline));
}

int sock_send(struct sock *sock, size_t len, void const *buf)
{
    struct msg *msg = msend(sock->stream, len, buf, sock->last_send);
    if (!msg) return H3C_ENOMEM;
    sock->last_send = msg;
    return H3C_OK;
}

int sock_recv(struct sock *sock, size_t len, void *buf)
{
    struct msg *msg = mrecv(sock->stream, len, buf, sock->last_recv);
    if (!msg) return H3C_ENOMEM;
    sock->last_recv = msg;
    return H3C_OK;
}

void sock_close(struct sock *sock)
{
    if (sock->stream)
    {
        nng_stream_close(sock->stream);
        nng_stream_free(sock->stream);
    }
    if (sock->aio) nng_aio_free(sock->aio);
    sock->stream = NULL;
    sock->aio = NULL;
}

void sock_del(struct sock *sock)
{
    if (!sock) return;
    sock_close(sock);
    free(sock);
}
