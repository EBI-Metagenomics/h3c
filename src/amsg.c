#include "amsg.h"
#include "h3c/code.h"
#include <nng/nng.h>
#include <stdint.h>

struct amsg
{
    size_t rem;
    struct nng_iov iov;
    struct nng_aio *upper_aio;
    struct nng_aio *lower_aio;
    struct nng_stream *s;
    void (*submit)(struct nng_stream *, struct nng_aio *);
};

static struct amsg *alloc(nng_stream *s,
                          void (*submit)(nng_stream *, nng_aio *), size_t len,
                          void *buf, void (*callb)(void *), void *arg);
static void destroy(struct amsg *x);
static void clean(struct amsg *x);
static void start(struct amsg *x);
static void callback(void *arg);

struct amsg *arecv(struct nng_stream *s, size_t len, void *data,
                   void (*callb)(void *), void *arg)
{
    struct amsg *x = alloc(s, &nng_stream_recv, len, data, callb, arg);
    if (!x) return x;
    start(x);
    return x;
}

int await(struct amsg *x)
{
    int rv;
    if (!x) return H3C_ENOMEM;
    nng_aio_wait(x->upper_aio);
    rv = nng_aio_result(x->upper_aio);
    clean(x);
    return rv;
}

void adel(struct amsg *x) { destroy(x); }

void acancel(struct amsg *x) { nng_aio_cancel(x->upper_aio); }

int aresult(struct amsg *x) { return nng_aio_result(x->upper_aio); }

static struct amsg *alloc(nng_stream *s,
                          void (*submit)(nng_stream *, nng_aio *), size_t len,
                          void *buf, void (*callb)(void *), void *arg)
{
    struct amsg *x;

    if ((x = nng_alloc(sizeof(*x))) == NULL) return NULL;
    if (nng_aio_alloc(&x->upper_aio, callb, arg) != 0)
    {
        destroy(x);
        return NULL;
    }
    if (nng_aio_alloc(&x->lower_aio, &callback, x) != 0)
    {
        destroy(x);
        return NULL;
    }

    // Upper should not take more than 30 seconds, lower not more than 5.
    nng_aio_set_timeout(x->upper_aio, 30000);
    nng_aio_set_timeout(x->lower_aio, 5000);

    nng_aio_begin(x->upper_aio);

    x->s = s;
    x->rem = len;
    x->iov.iov_buf = buf;
    x->iov.iov_len = len;
    x->submit = submit;

    return x;
}

static void clean(struct amsg *x)
{
    if (!x) return;
    if (x->lower_aio)
    {
        nng_aio_free(x->lower_aio);
        x->lower_aio = NULL;
    }
}

static void destroy(struct amsg *x)
{
    if (!x) return;
    clean(x);
    if (x->upper_aio)
    {
        nng_aio_free(x->upper_aio);
        x->upper_aio = NULL;
    }
    nng_free(x, sizeof(*x));
}

static void start(struct amsg *x)
{
    nng_aio_set_iov(x->lower_aio, 1, &x->iov);
    x->submit(x->s, x->lower_aio);
}

static void callback(void *arg)
{
    struct amsg *x = arg;
    int rv;
    size_t n;

    rv = nng_aio_result(x->lower_aio);
    if (rv != 0)
    {
        nng_aio_finish(x->upper_aio, rv);
        return;
    }
    n = nng_aio_count(x->lower_aio);

    x->rem -= n;

    if (x->rem == 0)
    {
        nng_aio_finish(x->upper_aio, 0);
        return;
    }

    start(x);
}
