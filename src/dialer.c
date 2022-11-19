#include "dialer.h"
#include "h3c/code.h"
#include "nngerr.h"
#include "timeout.h"
#include <nng/nng.h>
#include <stdio.h>
#include <stdlib.h>

struct dialer
{
    nng_stream_dialer *stream;
    nng_aio *aio;
};

struct dialer *dialer_new(char const *uri)
{
    struct dialer *dialer = malloc(sizeof(*dialer));
    if (!dialer) return NULL;
    dialer->stream = NULL;
    dialer->aio = NULL;
    if (nng_stream_dialer_alloc(&dialer->stream, uri))
    {
        free(dialer);
        return NULL;
    }
    return dialer;
}

int dialer_open(struct dialer *dialer, long deadline)
{
    int rc = H3C_OK;
    if ((rc = nngerr(nng_aio_alloc(&dialer->aio, NULL, NULL)))) goto cleanup;
    nng_aio_set_timeout(dialer->aio, timeout(deadline));
    nng_stream_dialer_dial(dialer->stream, dialer->aio);
    nng_aio_wait(dialer->aio);
    if ((rc = nngerr(nng_aio_result(dialer->aio)))) goto cleanup;
    return rc;

cleanup:
    dialer_close(dialer);
    return rc;
}

struct nng_stream *dialer_output(struct dialer *dialer)
{
    return nng_aio_get_output(dialer->aio, 0);
}

void dialer_close(struct dialer *dialer)
{
    if (dialer->aio) nng_aio_free(dialer->aio);
    dialer->aio = NULL;
}

void dialer_del(struct dialer *dialer)
{
    if (!dialer) return;
    if (dialer->stream) nng_stream_dialer_free(dialer->stream);
    dialer_close(dialer);
    free(dialer);
}
