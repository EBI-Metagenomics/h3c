#include "sock.h"
#include "answer.h"
#include "h3c/code.h"
#include "nngerr.h"
#include "request.h"
#include "timeout.h"
#include <nng/nng.h>
#include <stddef.h>
#include <stdlib.h>

struct sock *sock_new(void)
{
    struct sock *s = malloc(sizeof(*s));
    if (!s) return NULL;
    s->aio = NULL;
    s->stream = NULL;
    return s;
}

int sock_open(struct sock *s, struct nng_stream *stream)
{
    int rc = H3C_OK;
    if ((rc = nngerr(nng_aio_alloc(&s->aio, NULL, NULL)))) goto cleanup;
    s->stream = stream;
    return rc;

cleanup:
    sock_close(s);
    return rc;
}

void sock_set_deadline(struct sock *s, long deadline)
{
    nng_aio_set_timeout(s->aio, timeout(deadline));
}

static int send(struct nng_stream *, nng_aio *, size_t *sz, void const *buf);

int sock_send(struct sock *s, size_t len, void const *buf)
{
    while (len > 0)
    {
        size_t sent = len;
        int rc = send(s->stream, s->aio, &sent, buf);
        if (rc) return rc;
        len -= sent;
        buf += sent;
    }
    return H3C_OK;
}

static int recv(struct nng_stream *, nng_aio *aio, size_t *sz, void *buf);

int sock_recv(struct sock *s, size_t len, void *buf)
{
    while (len > 0)
    {
        size_t received = len;
        int rc = recv(s->stream, s->aio, &received, buf);
        if (rc) return rc;
        len -= received;
        buf += received;
    }
    return H3C_OK;
}

void sock_close(struct sock *s)
{
    if (s->stream)
    {
        nng_stream_close(s->stream);
        nng_stream_free(s->stream);
    }
    if (s->aio) nng_aio_free(s->aio);
    s->stream = NULL;
    s->aio = NULL;
}

void sock_del(struct sock *s)
{
    if (s) sock_close(s);
    free(s);
}

static int send(struct nng_stream *stream, nng_aio *aio, size_t *len,
                void const *buf)
{
    int rc = H3C_OK;
    nng_iov iov = {0};
    iov.iov_buf = (void *)buf;
    iov.iov_len = *len;
    if ((rc = nng_aio_set_iov(aio, 1, &iov))) return rc;
    nng_stream_send(stream, aio);
    nng_aio_wait(aio);
    *len = nng_aio_count(aio);
    return nngerr(nng_aio_result(aio));
}

static int recv(struct nng_stream *stream, nng_aio *aio, size_t *len, void *buf)
{
    int rc = H3C_OK;
    nng_iov iov = {0};
    iov.iov_buf = buf;
    iov.iov_len = *len;
    if ((rc = nng_aio_set_iov(aio, 1, &iov))) return rc;
    nng_stream_recv(stream, aio);
    nng_aio_wait(aio);
    *len = nng_aio_count(aio);
    return nngerr(nng_aio_result(aio));
}
