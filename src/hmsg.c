#include "hmsg.h"
#include "amsg.h"
#include "answer.h"
#include "h3c/code.h"
#include "hmmd/hmmd.h"
#include "nnge.h"
#include <nng/nng.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum state
{
    STATUS,
    DATA,
};

struct hmsg
{
    enum state state;
    struct nng_stream *stream;
    struct nng_aio *aio;
    struct answer *ans;
    struct amsg *amsg0;
    struct amsg *amsg1;
};

static int read_code(int c)
{
    if (c == NNG_ETIMEDOUT) return H3C_ETIMEDOUT;
    if (c == NNG_ECANCELED) return H3C_ECANCELED;
    return c;
}

static void callback(void *arg)
{
    struct hmsg *x = arg;
    int rc = nng_aio_result(x->aio);
    if (rc != 0)
    {
        nng_aio_finish(x->aio, read_code(rc));
        return;
    }

    if (x->state == STATUS)
    {
        struct hmmd_status const *status = answer_status_parse(x->ans);
        size_t size = status->msg_size;

        if ((rc = answer_setup_size(x->ans, size)))
        {
            nng_aio_finish(x->aio, H3C_ENOMEM);
            return;
        }

        void *data = answer_data(x->ans);
        x->amsg1 = arecv(x->stream, size, data, &callback, x);
        x->state = DATA;
    }
    else if (x->state == DATA)
    {
        if (!answer_status(x->ans)->status)
        {
            if ((rc = answer_parse(x->ans)))
            {
                nng_aio_finish(x->aio, rc);
                return;
            }
        }
        nng_aio_finish(x->aio, H3C_OK);
    }
}

struct hmsg *hrecv(struct nng_stream *stream, struct answer *ans, int timeout)
{
    struct hmsg *x = malloc(sizeof(*x));
    if (!x) return NULL;

    x->state = STATUS;
    x->stream = NULL;
    x->aio = NULL;
    x->ans = NULL;
    x->amsg0 = NULL;
    x->amsg1 = NULL;

    x->stream = stream;
    if ((!nng_aio_alloc(&x->aio, NULL, NULL)))
    {
        hdel(x);
        return NULL;
    }
    nng_aio_set_timeout(x->aio, timeout);
    nng_aio_begin(x->aio);

    x->ans = ans;

    void *data = answer_status_data(ans);
    size_t size = answer_status_size();
    if (!(x->amsg0 = arecv(stream, size, data, &callback, x)))
    {
        hdel(x);
        return NULL;
    }

    return x;
}

int hwait(struct hmsg *x)
{
    nng_aio_wait(x->aio);
    return nng_aio_result(x->aio);
}

void hdel(struct hmsg *x)
{
    if (!x) return;
    if (x->amsg1) adel(x->amsg1);
    if (x->amsg0) adel(x->amsg0);
    if (x->aio) nng_aio_free(x->aio);
    free(x);
}

void hcancel(struct hmsg *x)
{
    if (x->aio) nng_aio_cancel(x->aio);
}

int hresult(struct hmsg *x) { return nng_aio_result(x->aio); }

struct answer *houtput(struct hmsg *x) { return x->ans; }
