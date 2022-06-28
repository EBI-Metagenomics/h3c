#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#ifdef HAVE_NETINET_IN_H
/* On FreeBSD, you need netinet/in.h for struct sockaddr_in */
#include <netinet/in.h>
#endif
/* On OpenBSD, netinet/in.h is required for (must precede) arpa/inet.h  */
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "addr.h"
#include "answer.h"
#include "h3client/h3client.h"
#include "hmmd/hmmd.h"
#include "request.h"

struct conn
{
    struct addr addr;
    int sockfd;

    struct request *request;
    struct answer *answer;

} conn = {0};

enum h3c_rc h3c_open(char const *ip, uint16_t port)
{
    enum h3c_rc rc = H3C_OK;
    conn.request = 0;
    conn.answer = 0;

    if (!(conn.request = request_new()))
    {
        rc = H3C_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    if (!(conn.answer = answer_new()))
    {
        rc = H3C_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    if ((rc = addr_setup(&conn.addr, IPV4, ip, port))) goto cleanup;

    if ((conn.sockfd = socket(addr_family(&conn.addr), SOCK_STREAM, 0)) == -1)
    {
        rc = H3C_FAILED_CREATE_SOCKET;
        goto cleanup;
    }

    if (connect(conn.sockfd, (struct sockaddr *)&conn.addr,
                sizeof(conn.addr)) == -1)
    {

        rc = H3C_FAILED_CONNECT;
        goto cleanup;
    }

    return rc;

cleanup:
    if (conn.request) request_del(conn.request);
    if (conn.answer) answer_del(conn.answer);

    return rc;
}

static enum h3c_rc writen(int fd, void const *buf, size_t count);
static enum h3c_rc readn(int fd, void *buf, size_t count);

enum h3c_rc h3c_call(char const *args, FILE *fasta, struct h3c_result *result)
{
    enum h3c_rc rc = H3C_OK;

    if ((rc = request_set_args(conn.request, args))) goto cleanup;
    if ((rc = request_set_seqs(conn.request, fasta))) goto cleanup;

    size_t size = request_size(conn.request);
    if ((rc = writen(conn.sockfd, request_data(conn.request), size)))
        goto cleanup;

    void *data = answer_status_data(conn.answer);
    size = answer_status_size();
    if ((rc = readn(conn.sockfd, data, size))) goto cleanup;

    struct hmmd_status const *status = answer_status_parse(conn.answer);

    size = status->msg_size;
    if ((rc = answer_ensure(conn.answer, size))) goto cleanup;

    data = answer_data(conn.answer);
    if ((rc = readn(conn.sockfd, data, size))) goto cleanup;

    if (!status->status)
    {
        if ((rc = answer_parse(conn.answer))) goto cleanup;
        if ((rc = answer_copy(conn.answer, result))) goto cleanup;
    }

cleanup:
    return rc;
}

enum h3c_rc h3c_close(void)
{
    request_del(conn.request);
    answer_del(conn.answer);
    return close(conn.sockfd) ? H3C_FAILED_CLOSE : H3C_OK;
}

static enum h3c_rc writen(int fd, void const *buf, size_t count)
{
    while (count > 0)
    {
        ssize_t n = write(fd, buf, count);
        if (n == -1)
        {
            if (errno == EINTR)
            {
                n = 0;
                errno = 0;
            }
            else
            {
                errno = 0;
                return H3C_FAILED_WRITE_SOCKET;
            }
        }
        count -= n;
        buf += n;
    }

    return H3C_OK;
}

static enum h3c_rc readn(int fd, void *buf, size_t count)
{
    while (count > 0)
    {
        ssize_t n = read(fd, buf, count);
        if (n == -1)
        {
            if (errno == EINTR)
            {
                n = 0;
                errno = 0;
            }
            else
            {
                errno = 0;
                return H3C_FAILED_READ_SOCKET;
            }
        }

        count -= n;
        buf += n;
    }

    return count == 0 ? H3C_OK : H3C_FAILED_READ_SOCKET;
}
