#include "hmsg.h"
#include "amsg.h"
#include "answer.h"
#include "h3c/code.h"
#include "hmmd/hmmd.h"
#include "nnge.h"
#include "timeout.h"
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
    long deadline;
};

static int read_code(int c)
{
    if (c == NNG_ETIMEDOUT) return H3C_ETIMEDOUT;
    if (c == NNG_ECANCELED) return H3C_ECANCELED;
    return c;
}

static void callb3(void *arg)
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
        fprintf(stderr, "callb3: STATUS\n");
        struct hmmd_status const *status = answer_status_parse(x->ans);
        size_t size = status->msg_size;

        if ((rc = answer_setup_size(x->ans, size)))
        {
            nng_aio_finish(x->aio, H3C_ENOMEM);
            return;
        }

        void *data = answer_data(x->ans);
        if (!(x->amsg1 = arecv(x->stream, size, data, &callb3, x, x->deadline)))
        {
            nng_aio_finish(x->aio, H3C_ENOMEM);
            return;
        }
        astart(x->amsg1);
        x->state = DATA;
    }
    else if (x->state == DATA)
    {
        fprintf(stderr, "callb3: DATA 1\n");
        if (!answer_status(x->ans)->status)
        {
            fprintf(stderr, "callb3: DATA 2\n");
            if ((rc = answer_parse(x->ans)))
            {
                fprintf(stderr, "callb3: DATA 3\n");
                nng_aio_finish(x->aio, rc);
                return;
            }
        }
        nng_aio_finish(x->aio, H3C_OK);
    }
}

struct hmsg *hrecv(struct nng_stream *stream, struct answer *ans,
                   void (*callb)(void *), void *arg, long deadline)
{
    struct hmsg *x = malloc(sizeof(*x));
    if (!x) return NULL;

    x->state = STATUS;
    x->stream = NULL;
    x->aio = NULL;
    x->ans = NULL;
    x->amsg0 = NULL;
    x->amsg1 = NULL;
    x->deadline = deadline;

    x->stream = stream;
    if (nng_aio_alloc(&x->aio, callb, arg))
    {
        hdel(x);
        return NULL;
    }
    nng_aio_set_timeout(x->aio, timeout(deadline));
    nng_aio_begin(x->aio);

    x->ans = ans;

    void *data = answer_status_data(ans);
    size_t size = answer_status_size();
    char *b = data;
    for (int i = 0; i < (int)size; ++i)
        b[i] = 0;
    if (!(x->amsg0 = arecv(stream, size, data, &callb3, x, deadline)))
    {
        hdel(x);
        return NULL;
    }

    return x;
}

void hstart(struct hmsg *x) { astart(x->amsg0); }

int hwait(struct hmsg *x)
{
    nng_aio_wait(x->aio);
    return nng_aio_result(x->aio);
}

void hcancel(struct hmsg *x) { nng_aio_cancel(x->aio); }

void hstop(struct hmsg *x) { nng_aio_stop(x->aio); }

void hdel(struct hmsg *x)
{
    if (!x) return;
    if (x->amsg1) adel(x->amsg1);
    if (x->amsg0) adel(x->amsg0);
    if (x->aio) nng_aio_free(x->aio);
    free(x);
}

int hresult(struct hmsg *x) { return nng_aio_result(x->aio); }

struct answer *houtput(struct hmsg *x) { return x->ans; }
