#include "dialer.h"
#include "h3c/code.h"
#include "h3c/stream.h"
#include "itoa.h"
#include "nng/nng.h"
#include "nnge.h"
#include "stream.h"
#include "timeout.h"
#include <stdlib.h>
#include <string.h>

struct h3c_dialer
{
    nng_stream_dialer *stream;
    nng_aio *aio;
    struct h3c_stream *h3c_stream;
};

struct h3c_dialer *h3c_dialer_new(char const *ip, int port)
{
    struct h3c_dialer *dialer = malloc(sizeof(*dialer));
    if (!dialer) return NULL;

    char uri[512] = "tcp://";
    strncat(uri, ip, sizeof(uri) - strlen(uri) - 27);
    strcat(uri, ":");
    h3c_itoa(uri + strlen(uri), port);

    dialer->stream = NULL;
    dialer->aio = NULL;
    dialer->h3c_stream = NULL;

    if (nng_stream_dialer_alloc(&dialer->stream, uri))
    {
        free(dialer);
        return NULL;
    }

    if (nng_aio_alloc(&dialer->aio, NULL, NULL))
    {
        nng_stream_dialer_close(dialer->stream);
        nng_stream_dialer_free(dialer->stream);
        free(dialer);
        return NULL;
    }

    return dialer;
}

int h3c_dialer_dial(struct h3c_dialer *x, long deadline)
{
    nng_aio_set_timeout(x->aio, h3c_timeout(deadline));
    nng_stream_dialer_dial(x->stream, x->aio);
    nng_aio_wait(x->aio);

    int rc = h3c_nnge(nng_aio_result(x->aio));
    if (rc) return rc;

    struct nng_stream *s = nng_aio_get_output(x->aio, 0);
    struct h3c_stream *stream = h3c_stream_new(s);
    if (!stream)
    {
        nng_stream_close(s);
        nng_stream_free(s);
        return H3C_ENOMEM;
    }
    x->h3c_stream = stream;
    return H3C_OK;
}

struct h3c_stream *h3c_dialer_stream(struct h3c_dialer *x)
{
    struct h3c_stream *s = x->h3c_stream;
    x->h3c_stream = NULL;
    return s;
}

void h3c_dialer_del(struct h3c_dialer *x)
{
    if (!x) return;

    if (x->h3c_stream) h3c_stream_del(x->h3c_stream);
    if (x->aio) nng_aio_free(x->aio);
    if (x->stream)
    {
        nng_stream_dialer_close(x->stream);
        nng_stream_dialer_free(x->stream);
    }

    free(x);
}
