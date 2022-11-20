#include "msg.h"
#include "h3c/code.h"
#include <nng/nng.h>
#include <stdint.h>
#include <string.h>

struct msg
{
    size_t rem;
    int len;
    struct nng_iov iov[8];
    struct nng_aio *upper_aio;
    struct nng_aio *lower_aio;
    struct nng_stream *s;
    void (*submit)(struct nng_stream *, struct nng_aio *);
};

static struct msg *alloc(nng_stream *s, void (*submit)(nng_stream *, nng_aio *),
                         int len, struct nng_iov *, void (*callb)(void *),
                         void *arg);
static void destroy(struct msg *x);
static void start(struct msg *x);
static void callback(void *arg);

struct msg *msend(struct nng_stream *s, int len, struct nng_iov *iov)
{
    struct msg *x = alloc(s, &nng_stream_send, len, iov, NULL, NULL);
    if (!x) return x;
    start(x);
    return x;
}

struct msg *mrecv(struct nng_stream *s, int len, struct nng_iov *iov)
{
    struct msg *x = alloc(s, &nng_stream_recv, len, iov, NULL, NULL);
    if (!x) return x;
    start(x);
    return x;
}

struct msg *mrecv_callb(struct nng_stream *s, size_t size, void *data,
                        void (*callb)(void *), void *arg)
{
    struct nng_iov iov = {.iov_buf = data, .iov_len = size};
    struct msg *x = alloc(s, &nng_stream_recv, 1, &iov, callb, arg);
    if (!x) return x;
    start(x);
    return x;
}

int mwait(struct msg *x)
{
    int rv;
    if (!x) return H3C_ENOMEM;
    nng_aio_wait(x->upper_aio);
    rv = nng_aio_result(x->upper_aio);
    destroy(x);
    return rv;
}

static struct msg *alloc(nng_stream *s, void (*submit)(nng_stream *, nng_aio *),
                         int len, struct nng_iov *iov, void (*callb)(void *),
                         void *arg)
{
    struct msg *x;

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
    x->rem = 0;
    for (int i = 0; i < len; ++i)
        x->rem += iov[i].iov_len;
    x->len = len;
    memcpy(x->iov, iov, len * sizeof(*iov));
    x->submit = submit;

    return x;
}

static void destroy(struct msg *x)
{
    if (!x) return;
    if (x->upper_aio) nng_aio_free(x->upper_aio);
    if (x->lower_aio) nng_aio_free(x->lower_aio);
    nng_free(x, sizeof(*x));
}

static void start(struct msg *x)
{
    nng_aio_set_iov(x->lower_aio, x->len, x->iov);
    x->submit(x->s, x->lower_aio);
}

static void callback(void *arg)
{
    struct msg *x = arg;
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
