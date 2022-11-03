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
#include "h3c/h3c.h"
#include "hmmd/hmmd.h"
#include "request.h"

struct conn
{
    struct addr addr;
    int sockfd;

    struct request *request;
    struct answer *answer;

} conn = {0};

int h3c_open(char const *ip, uint16_t port)
{
    int rc = H3C_OK;
    conn.sockfd = -1;
    conn.request = NULL;
    conn.answer = NULL;

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
    if (conn.sockfd < 0) close(conn.sockfd);
    conn.answer = NULL;
    conn.request = NULL;
    conn.sockfd = -1;
    return rc;
}

static int writen(int fd, void const *buf, size_t count);
static int readn(int fd, void *buf, size_t count);
static int recv_answer(struct h3c_result *result);

int h3c_begin(char const *args)
{
    int rc = H3C_OK;
    if ((rc = writen(conn.sockfd, "@", 1))) return rc;
    if ((rc = writen(conn.sockfd, args, strlen(args)))) return rc;
    if ((rc = writen(conn.sockfd, "\n", 1))) return rc;
    return rc;
}

int h3c_send(char const *seq) { return writen(conn.sockfd, seq, strlen(seq)); }

int h3c_end(struct h3c_result *result)
{
    int rc = H3C_OK;
    if ((rc = writen(conn.sockfd, "//", 2))) return rc;
    return recv_answer(result);
}

#define READ_SIZE 4096

int h3c_callf(char const *args, FILE *fasta, struct h3c_result *result)
{
    int rc = H3C_OK;
    if ((rc = h3c_begin(args))) return rc;

    static char buf[READ_SIZE] = {0};
    while (fgets(buf, READ_SIZE, fasta))
    {
        if ((rc = h3c_send(buf))) return rc;
    }
    if (!feof(fasta)) rc = H3C_FAILED_READ_FILE;
    if (rc) return rc;

    return h3c_end(result);
}

int h3c_close(void)
{
    int rc = H3C_OK;
    if (conn.request) request_del(conn.request);
    if (conn.answer) answer_del(conn.answer);
    if (conn.sockfd < 0) rc = close(conn.sockfd) ? H3C_FAILED_CLOSE : H3C_OK;
    conn.answer = NULL;
    conn.request = NULL;
    conn.sockfd = -1;
    return rc;
}

static int recv_answer(struct h3c_result *result)
{
    int rc = H3C_OK;
    void *data = answer_status_data(conn.answer);
    size_t size = answer_status_size();
    if ((rc = readn(conn.sockfd, data, size))) goto cleanup;

    struct hmmd_status const *status = answer_status_parse(conn.answer);

    size = status->msg_size;
    if ((rc = answer_setup_size(conn.answer, size))) goto cleanup;

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

static int writen(int fd, void const *buf, size_t count)
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

static int readn(int fd, void *buf, size_t count)
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
