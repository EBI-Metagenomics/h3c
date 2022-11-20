#include "dialer.h"
#include "h3c/code.h"
#include "nnge.h"
#include "sock.h"
#include "timeout.h"
#include <nng/nng.h>
#include <stdio.h>
#include <stdlib.h>

struct dialer
{
    nng_stream_dialer *stream;
    nng_aio *aio;
    struct sock *sock;
};

struct dialer *dialer_new(char const *uri)
{
    struct dialer *dialer = malloc(sizeof(*dialer));
    if (!dialer) return NULL;

    dialer->stream = NULL;
    dialer->aio = NULL;
    dialer->sock = NULL;

    if (nng_stream_dialer_alloc(&dialer->stream, uri))
    {
        free(dialer);
        return NULL;
    }

    if (nng_aio_alloc(&dialer->aio, NULL, NULL))
    {
        nng_stream_dialer_free(dialer->stream);
        free(dialer);
        return NULL;
    }

    return dialer;
}

int dialer_dial(struct dialer *dialer, long deadline)
{
    nng_aio_set_timeout(dialer->aio, timeout(deadline));
    nng_stream_dialer_dial(dialer->stream, dialer->aio);
    nng_aio_wait(dialer->aio);

    int rc = nnge(nng_aio_result(dialer->aio));
    if (rc) return rc;

    return (dialer->sock = sock_new()) ? H3C_OK : H3C_ENOMEM;
}

struct sock *dialer_sock(struct dialer *dialer)
{
    struct sock *sock = dialer->sock;
    dialer->sock = NULL;
    sock_open(sock, nng_aio_get_output(dialer->aio, 0));
    return sock;
}

void dialer_del(struct dialer *dialer)
{
    if (!dialer) return;

    if (dialer->sock) sock_del(dialer->sock);
    if (dialer->aio) nng_aio_free(dialer->aio);
    if (dialer->stream) nng_stream_dialer_free(dialer->stream);

    free(dialer);
}
