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
#include "hmmd.h"
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

static bool writen(int fd, void const *buf, size_t count);
static bool readn(int fd, void *buf, size_t count);

enum h3c_rc h3c_call(char const *args, FILE *fasta)
{
    enum h3c_rc rc = H3C_OK;

    if ((rc = request_set_args(conn.request, args))) goto cleanup;
    if ((rc = request_set_seqs(conn.request, fasta))) goto cleanup;

    size_t size = request_size(conn.request);
    if (!writen(conn.sockfd, request_data(conn.request), size)) return false;

    void *data = answer_status_data(conn.answer);
    size = answer_status_size();
    if (!readn(conn.sockfd, data, size)) return false;

    struct hmmd_status const *status = answer_status_unpack(conn.answer);

    size = status->msg_size;
    if (!answer_ensure(conn.answer, size)) return false;

    data = answer_data(conn.answer);
    if (!readn(conn.sockfd, data, size)) return false;

    if (!status->status)
    {
        // if (!answer_unpack(conn.answer)) return false;
    }

cleanup:
    return rc;
}

enum h3c_rc h3c_close(void)
{
    return close(conn.sockfd) ? H3C_FAILED_CLOSE : H3C_OK;
}

static bool writen(int fd, void const *buf, size_t count)
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
                return false;
            }
        }
        count -= n;
        buf += n;
    }

    return true;
}

static bool readn(int fd, void *buf, size_t count)
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
                return false;
            }
        }

        count -= n;
        buf += n;
    }

    return count == 0;
}
