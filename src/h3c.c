#include "h3c/h3c.h"
#include "answer.h"
#include "dillerr.h"
#include "hmmd/hmmd.h"
#include "request.h"
#include <errno.h>
#include <libdill.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct conn
{
    struct ipaddr addr;
    int fd;

    struct request *request;
    struct answer *answer;

} conn = {0};

int h3c_open(char const *ip, int port, long deadline)
{
    int rc = H3C_OK;
    conn.fd = -1;
    conn.request = NULL;
    conn.answer = NULL;

    if (!(conn.request = request_new()))
    {
        rc = H3C_NOMEM;
        goto cleanup;
    }

    if (!(conn.answer = answer_new()))
    {
        rc = H3C_NOMEM;
        goto cleanup;
    }

    if ((rc = ipaddr_remote(&conn.addr, ip, port, IPADDR_IPV4, deadline)))
    {
        rc = dillerr(errno);
        goto cleanup;
    }

    if ((conn.fd = tcp_connect(&conn.addr, deadline)) < 0)
    {
        rc = dillerr(errno);
        goto cleanup;
    }

    return rc;

cleanup:
    if (conn.request) request_del(conn.request);
    if (conn.answer) answer_del(conn.answer);
    if (conn.fd >= 0) tcp_close(conn.fd, deadline);
    conn.answer = NULL;
    conn.request = NULL;
    conn.fd = -1;
    return rc;
}

int h3c_close(long deadline)
{
    int rc = H3C_OK;
    if (conn.request) request_del(conn.request);
    if (conn.answer) answer_del(conn.answer);
    if (conn.fd >= 0)
        rc = tcp_close(conn.fd, deadline) ? dillerr(errno) : H3C_OK;
    conn.answer = NULL;
    conn.request = NULL;
    conn.fd = -1;
    return rc;
}

static int writen(int fd, void const *buf, size_t sz, long deadline);
static int recv_answer(struct h3c_result *result, long deadline);

int h3c_begin(char const *args, long deadline)
{
    int rc = H3C_OK;
    if ((rc = writen(conn.fd, "@", 1, deadline))) return rc;
    if ((rc = writen(conn.fd, args, strlen(args), deadline))) return rc;
    if ((rc = writen(conn.fd, "\n", 1, deadline))) return rc;
    return rc;
}

int h3c_put(char const *seq, long deadline)
{
    return writen(conn.fd, seq, strlen(seq), deadline);
}

int h3c_end(struct h3c_result *result, long deadline)
{
    int rc = H3C_OK;
    if ((rc = writen(conn.fd, "//", 2, deadline))) return rc;
    return recv_answer(result, deadline);
}

#define READ_SIZE 4096

int h3c_send(char const *args, FILE *fasta, struct h3c_result *result,
             long deadline)
{
    int rc = H3C_OK;
    if ((rc = h3c_begin(args, deadline))) return rc;

    char buf[READ_SIZE] = {0};
    while (fgets(buf, READ_SIZE, fasta))
    {
        if ((rc = h3c_put(buf, deadline))) return rc;
    }
    if (!feof(fasta)) rc = H3C_FAILED_READ_FILE;
    if (rc) return rc;

    return h3c_end(result, deadline);
}

long h3c_now(void) { return (long)now(); }

static int writen(int fd, void const *buf, size_t sz, long deadline)
{
    return bsend(fd, buf, sz, deadline) ? dillerr(errno) : H3C_OK;
}

static int readn(int fd, void *buf, size_t sz, long deadline)
{
    return brecv(fd, buf, sz, deadline) ? dillerr(errno) : H3C_OK;
}

static int recv_answer(struct h3c_result *result, long deadline)
{
    int rc = H3C_OK;
    void *data = answer_status_data(conn.answer);
    size_t size = answer_status_size();
    if ((rc = readn(conn.fd, data, size, deadline))) goto cleanup;

    struct hmmd_status const *status = answer_status_parse(conn.answer);

    size = status->msg_size;
    if ((rc = answer_setup_size(conn.answer, size))) goto cleanup;

    data = answer_data(conn.answer);
    if ((rc = readn(conn.fd, data, size, deadline))) goto cleanup;

    if (!status->status)
    {
        if ((rc = answer_parse(conn.answer))) goto cleanup;
        if ((rc = answer_copy(conn.answer, result))) goto cleanup;
    }

cleanup:
    return rc;
}
