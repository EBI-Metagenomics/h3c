#include "amsg.h"
#include "h3c/code.h"
#include "timeout.h"
#include <nng/nng.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct amsg
{
    int len;
    struct nng_iov iov[8];
    struct nng_aio *upper_aio;
    struct nng_aio *lower_aio;
    struct nng_stream *s;
    void (*submit)(struct nng_stream *, struct nng_aio *);
};

static struct amsg *alloc(nng_stream *s,
                          void (*submit)(nng_stream *, nng_aio *), int len,
                          struct nng_iov *, void (*callb)(void *), void *arg,
                          long deadline);
static void destroy2(struct amsg *x);
static void start(struct amsg *x);
static void callb4(void *arg);

struct amsg *asend(struct nng_stream *s, int len, struct nng_iov *iov,
                   void (*callb)(void *), void *arg, long deadline)
{
    return alloc(s, &nng_stream_send, len, iov, callb, arg, deadline);
}

struct amsg *arecv(struct nng_stream *s, size_t len, void *data,
                   void (*callb)(void *), void *arg, long deadline)
{
    fprintf(stderr, "arecv len : %ld\n", len);
    fprintf(stderr, "arecv data: %p\n", data);
    fprintf(stderr, "\n");
    struct nng_iov iov = {.iov_buf = data, .iov_len = len};
    return alloc(s, &nng_stream_recv, 1, &iov, callb, arg, deadline);
}

void astart(struct amsg *x) { start(x); }

int await(struct amsg *x)
{
    nng_aio_wait(x->upper_aio);
    return nng_aio_result(x->upper_aio);
}

void adel(struct amsg *x) { destroy2(x); }

void acancel(struct amsg *x) { nng_aio_cancel(x->upper_aio); }

int aresult(struct amsg *x) { return nng_aio_result(x->upper_aio); }

static int consume(int len, struct nng_iov *iov, size_t amount);

static struct amsg *alloc(nng_stream *s,
                          void (*submit)(nng_stream *, nng_aio *), int len,
                          struct nng_iov *iov, void (*callb)(void *), void *arg,
                          long deadline)
{
    struct amsg *x;

    if ((x = nng_alloc(sizeof(*x))) == NULL) return NULL;
    if (nng_aio_alloc(&x->upper_aio, callb, arg) != 0)
    {
        destroy2(x);
        return NULL;
    }
    if (nng_aio_alloc(&x->lower_aio, &callb4, x) != 0)
    {
        destroy2(x);
        return NULL;
    }

    fprintf(stderr, "amsg timeout: %d\n", timeout(deadline));
    nng_aio_set_timeout(x->upper_aio, timeout(deadline));
    nng_aio_set_timeout(x->lower_aio, 5000);

    nng_aio_begin(x->upper_aio);

    x->s = s;
    x->len = len;
    memcpy(x->iov, iov, len * sizeof(*iov));
    x->submit = submit;

    return x;
}

static void destroy2(struct amsg *x)
{
    fprintf(stderr, "amsg destroy\n");
    if (!x) return;
    if (x->lower_aio)
    {
        nng_aio_free(x->lower_aio);
        x->lower_aio = NULL;
    }
    if (x->upper_aio)
    {
        nng_aio_free(x->upper_aio);
        x->upper_aio = NULL;
    }
    nng_free(x, sizeof(*x));
}

static void start(struct amsg *x)
{
    nng_aio_set_iov(x->lower_aio, x->len, x->iov);
    x->submit(x->s, x->lower_aio);
}

static void callb4(void *arg)
{
    struct amsg *x = arg;
    int rv;
    size_t n;

    rv = nng_aio_result(x->lower_aio);
    fprintf(stderr, "callb4 1: %d\n", rv);
    if (rv != 0)
    {
        nng_aio_finish(x->upper_aio, rv);
        return;
    }
    n = nng_aio_count(x->lower_aio);
    fprintf(stderr, "callb4 2: %ld\n", n);

    if (!consume(x->len, x->iov, n))
    {
        nng_aio_finish(x->upper_aio, 0);
        return;
    }

    start(x);
}

static int consume(int len, struct nng_iov *iov, size_t amount)
{
    int rem = len;
    for (int i = 0; i < len; ++i)
    {
        size_t sz = amount < iov->iov_len ? amount : iov->iov_len;
        iov->iov_len -= sz;
        iov->iov_buf += sz;
        amount -= sz;
        rem -= iov->iov_len == 0;
        ++iov;
    }
    return rem;
}
