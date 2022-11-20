#include "msg.h"
#include "amsg.h"
#include "answer.h"
#include "h3c/code.h"
#include "hmsg.h"
#include "timeout.h"
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include <stdlib.h>
#include <string.h>

struct msg *msg_new(struct nng_stream *stream)
{
    struct msg *x = malloc(sizeof(*x));
    if (!x) return x;

    x->stream = stream;
    x->state = 0;
    x->deadline = 0;
    x->aio = NULL;
    x->ans = NULL;
    x->send_amsg = NULL;
    x->recv_hmsg = NULL;
    x->mtx = NULL;
    cco_node_init(&x->node);

    if (nng_aio_alloc(&x->aio, NULL, NULL))
    {
        free(x);
        return NULL;
    }

    if (!(x->ans = answer_new()))
    {
        nng_aio_free(x->aio);
        free(x);
        return NULL;
    }

    if (nng_mtx_alloc(&x->mtx))
    {
        answer_del(x->ans);
        nng_aio_free(x->aio);
        free(x);
        return NULL;
    }

    return x;
}

static void callback(void *arg);

int msg_start(struct msg *x, char const *args, char const *seq, long deadline)
{
    struct nng_iov iov[5] = {
        {.iov_buf = "@", .iov_len = 1},
        {.iov_buf = (void *)args, .iov_len = strlen(args)},
        {.iov_buf = "\n", .iov_len = 1},
        {.iov_buf = (void *)seq, .iov_len = strlen(seq)},
        {.iov_buf = "//", .iov_len = 2},
    };
    x->state = SEND;
    x->deadline = deadline;

    nng_aio_set_timeout(x->aio, timeout(deadline));
    nng_aio_begin(x->aio);

    if (!(x->send_amsg = asend(x->stream, 5, iov, &callback, x, deadline)))
        return H3C_ENOMEM;

    astart(x->send_amsg);
    return H3C_OK;
}

int msg_wait(struct msg *x)
{
    nng_aio_wait(x->aio);
    return nng_aio_result(x->aio);
}

void msg_cancel(struct msg *x)
{
    nng_mtx_lock(x->mtx);
    if (x->recv_hmsg) hcancel(x->recv_hmsg);
    nng_mtx_unlock(x->mtx);
    acancel(x->send_amsg);
    nng_aio_cancel(x->aio);
}

void msg_stop(struct msg *x)
{
    nng_mtx_lock(x->mtx);
    if (x->recv_hmsg) hstop(x->recv_hmsg);
    nng_mtx_unlock(x->mtx);
    astop(x->send_amsg);
    nng_aio_stop(x->aio);
}

void msg_del(struct msg *x)
{
    if (!x) return;
    if (x->mtx) nng_mtx_free(x->mtx);
    if (x->ans) answer_del(x->ans);
    if (x->send_amsg) adel(x->send_amsg);
    nng_mtx_lock(x->mtx);
    if (x->recv_hmsg) hdel(x->recv_hmsg);
    nng_mtx_unlock(x->mtx);
    if (x->aio) nng_aio_free(x->aio);
    free(x);
}

struct answer *msg_answer(struct msg *x) { return x->ans; }

static void callback(void *arg)
{
    struct msg *x = arg;

    if (x->state == SEND)
    {
        int rc = aresult(x->send_amsg);
        if (rc)
        {
            nng_aio_finish(x->aio, rc);
            return;
        }

        nng_mtx_lock(x->mtx);
        x->recv_hmsg = hrecv(x->stream, x->ans, &callback, x, x->deadline);
        nng_mtx_unlock(x->mtx);
        if (!x->recv_hmsg)
        {
            nng_aio_finish(x->aio, H3C_ENOMEM);
            return;
        }

        x->state = RECV;
        hstart(x->recv_hmsg);
    }
    else if (x->state == RECV)
    {
        nng_aio_finish(x->aio, hresult(x->recv_hmsg));
    }
}
