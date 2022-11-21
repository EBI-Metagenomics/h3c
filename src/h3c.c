#define _POSIX_SOURCE 200809L
#include "h3c/h3c.h"
#include "answer.h"
#include "dialer.h"
#include "hmmd/hmmd.h"
#include "itoa.h"
#include "nnge.h"
#include "request.h"
#include "task.h"
#include <errno.h>
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static nng_stream_dialer *dialer = NULL;
static nng_aio *daio = NULL;
static nng_stream *stream = NULL;
static struct request *request = NULL;
static struct answer *answer = NULL;

static struct dialer *dia = NULL;
static struct task *task = NULL;

static int timeout(long deadline)
{
    int timeout = deadline - h3c_now();
    if (timeout < 0) timeout = 0;
    return timeout;
}

int h3c_open(char const *ip, int port, long deadline)
{
    int rc = H3C_OK;

    if (!(request = request_new()))
    {
        rc = H3C_ENOMEM;
        goto cleanup;
    }

    if (!(answer = answer_new()))
    {
        rc = H3C_ENOMEM;
        goto cleanup;
    }

    char uri[512] = "tcp://";
    strncat(uri, ip, sizeof(uri) - strlen(uri) - 27);
    strcat(uri, ":");
    itoa(uri + strlen(uri), port);

    if ((rc = nnge(nng_stream_dialer_alloc(&dialer, uri)))) goto cleanup;
    if ((rc = nnge(nng_aio_alloc(&daio, NULL, NULL)))) goto cleanup;
    nng_aio_set_timeout(daio, timeout(deadline));
    nng_stream_dialer_dial(dialer, daio);
    nng_aio_wait(daio);
    if ((rc = nnge(nng_aio_result(daio)))) goto cleanup;

    stream = nng_aio_get_output(daio, 0);

    dia = dialer_new(uri);
    dialer_dial(dia, deadline);
    if (!(task = task_new(dialer_stream(dia))))
    {
        rc = H3C_ENOMEM;
        goto cleanup;
    }

    return rc;

cleanup:
    h3c_close();
    return rc;
}

void h3c_close(void)
{
    if (dia) dialer_del(dia);
    if (task) task_del(task);
    if (stream)
    {
        nng_stream_close(stream);
        nng_stream_free(stream);
    }
    if (daio) nng_aio_free(daio);
    if (dialer) nng_stream_dialer_free(dialer);
    if (answer) answer_del(answer);
    if (request) request_del(request);
    dialer = NULL;
    dia = NULL;
    task = NULL;
    stream = NULL;
    daio = NULL;
    dialer = NULL;
    answer = NULL;
    request = NULL;
}

int h3c_send(char const *args, char const *seq, long deadline)
{
    return task_put(task, args, seq, deadline);
}

void h3c_wait(void) { task_wait(task); }

int h3c_pop(struct h3c_result *r) { return task_pop(task, r); }

static int send1(nng_aio *aio, void const *buf, size_t *sz)
{
    int rc = H3C_OK;
    nng_iov iov = {0};
    iov.iov_buf = (void *)buf;
    iov.iov_len = *sz;
    if ((rc = nng_aio_set_iov(aio, 1, &iov))) return rc;
    nng_stream_send(stream, aio);
    nng_aio_wait(aio);
    *sz = nng_aio_count(aio);
    return nnge(nng_aio_result(aio));
}

static int send(nng_aio *aio, void const *buf, size_t total)
{
    while (total > 0)
    {
        size_t sent = total;
        int rc = send1(aio, buf, &sent);
        if (rc) return rc;
        total -= sent;
        buf += sent;
    }
    return H3C_OK;
}

static int recv1(nng_aio *aio, void *buf, size_t *sz)
{
    int rc = H3C_OK;
    nng_iov iov = {0};
    iov.iov_buf = buf;
    iov.iov_len = *sz;
    if ((rc = nng_aio_set_iov(aio, 1, &iov))) return rc;
    nng_stream_recv(stream, aio);
    nng_aio_wait(aio);
    *sz = nng_aio_count(aio);
    return nnge(nng_aio_result(aio));
}

static int recv(nng_aio *aio, void *buf, size_t total)
{
    while (total > 0)
    {
        size_t received = total;
        int rc = recv1(aio, buf, &received);
        if (rc) return rc;
        total -= received;
        buf += received;
    }
    return H3C_OK;
}

static int recv_answer(struct h3c_result *result, long deadline);

int h3c_begin(char const *args, long deadline)
{
    int rc = H3C_OK;
    nng_aio *aio = NULL;
    if ((rc = nnge(nng_aio_alloc(&aio, NULL, NULL)))) return rc;
    nng_aio_set_timeout(aio, timeout(deadline));

    if ((rc = send(aio, "@", 1))) goto cleanup;
    if ((rc = send(aio, args, strlen(args)))) goto cleanup;
    if ((rc = send(aio, "\n", 1))) goto cleanup;

cleanup:
    nng_aio_free(aio);
    return rc;
}

int h3c_put(char const *seq, long deadline)
{
    int rc = H3C_OK;
    nng_aio *aio = NULL;
    if ((rc = nnge(nng_aio_alloc(&aio, NULL, NULL)))) return rc;
    nng_aio_set_timeout(aio, timeout(deadline));
    rc = send(aio, seq, strlen(seq));
    nng_aio_free(aio);
    return rc;
}

int h3c_end(struct h3c_result *result, long deadline)
{
    int rc = H3C_OK;
    nng_aio *aio = NULL;
    if ((rc = nnge(nng_aio_alloc(&aio, NULL, NULL)))) return rc;
    nng_aio_set_timeout(aio, timeout(deadline));

    if ((rc = send(aio, "//", 2))) goto cleanup;
    rc = recv_answer(result, deadline);

cleanup:
    nng_aio_free(aio);
    return rc;
}

#define READ_SIZE 4096

int h3c_sendf(char const *args, FILE *fasta, struct h3c_result *result,
              long deadline)
{
    int rc = H3C_OK;
    if ((rc = h3c_begin(args, deadline))) return rc;

    char buf[READ_SIZE] = {0};
    while (fgets(buf, READ_SIZE, fasta))
    {
        if ((rc = h3c_put(buf, deadline))) return rc;
    }
    if (!feof(fasta)) rc = H3C_EUNEOF;
    if (rc) return rc;

    return h3c_end(result, deadline);
}

long h3c_now(void) { return (long)nng_clock(); }

static int recv_answer(struct h3c_result *result, long deadline)
{
    int rc = H3C_OK;
    nng_aio *aio = NULL;
    if ((rc = nnge(nng_aio_alloc(&aio, NULL, NULL)))) return rc;
    nng_aio_set_timeout(aio, timeout(deadline));

    void *data = answer_status_data(answer);
    size_t size = answer_status_size();
    if ((rc = recv(aio, data, size))) goto cleanup;

    struct hmmd_status const *status = answer_status_parse(answer);

    size = status->msg_size;
    if ((rc = answer_setup_size(answer, size))) goto cleanup;

    data = answer_data(answer);
    if ((rc = recv(aio, data, size))) goto cleanup;

    if (!status->status)
    {
        if ((rc = answer_parse(answer))) goto cleanup;
        if ((rc = answer_copy(answer, result))) goto cleanup;
    }

cleanup:
    nng_aio_free(aio);
    return rc;
}
