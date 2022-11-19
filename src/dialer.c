#include "dialer.h"
#include "h3c/code.h"
#include "nngerr.h"
#include "timeout.h"
#include <nng/nng.h>
#include <stdio.h>

static nng_stream_dialer *stream = NULL;
static nng_aio *aio = NULL;

static void callb(void *arg)
{
    (void)arg;
    fprintf(stderr, "Ponto 1\n");
    int rc = H3C_OK;
    fprintf(stderr, "Ponto 2\n");
    rc = nngerr(nng_aio_result(aio));
    fprintf(stderr, "Ponto 3\n");
    struct nng_stream *s = nng_aio_get_output(aio, 0);
    fprintf(stderr, "Ponto 4\n");
}

int dialer_open(char const *uri, int num_connections, long deadline)
{
    int rc = H3C_OK;
    stream = NULL;
    aio = NULL;
    if ((rc = nngerr(nng_stream_dialer_alloc(&stream, uri)))) goto cleanup;
    if ((rc = nngerr(nng_aio_alloc(&aio, &callb, NULL)))) goto cleanup;
    nng_aio_set_timeout(aio, timeout(deadline));
    for (int i = 0; i < num_connections; ++i)
        nng_stream_dialer_dial(stream, aio);
    return rc;

cleanup:
    dialer_close();
    return rc;
}

void dialer_close(void)
{
    if (aio) nng_aio_free(aio);
    if (stream) nng_stream_dialer_free(stream);
    aio = NULL;
    stream = NULL;
}
