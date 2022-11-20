#include "sock.h"
#include "answer.h"
#include "cco.h"
#include "h3c/code.h"
#include "msg.h"
#include "nnge.h"
#include "packet.h"
#include "request.h"
#include "timeout.h"
#include <nng/nng.h>
#include <stddef.h>
#include <stdlib.h>

struct sock
{
    struct nng_aio *aio;
    struct nng_stream *stream;
    struct cco_queue send_queue;
    struct cco_queue recv_queue;
};

struct sock *sock_new(void)
{
    struct sock *sock = malloc(sizeof(*sock));
    if (!sock) return NULL;

    if (nng_aio_alloc(&sock->aio, NULL, NULL))
    {
        free(sock);
        return NULL;
    }

    sock->stream = NULL;
    cco_queue_init(&sock->send_queue);
    cco_queue_init(&sock->recv_queue);

    return sock;
}

void sock_open(struct sock *sock, struct nng_stream *stream)
{
    sock->stream = stream;
}

void sock_set_deadline(struct sock *sock, long deadline)
{
    nng_aio_set_timeout(sock->aio, timeout(deadline));
}

int sock_send(struct sock *sock, int len, struct nng_iov *iov, void *arg)
{
    struct packet *packet = packet_new();
    if (!packet) return H3C_ENOMEM;

    if (!(packet->msg = msend(sock->stream, len, iov)))
    {
        packet_del(packet);
        return H3C_ENOMEM;
    }
    packet->arg = arg;

    cco_queue_put(&sock->send_queue, &packet->node);
    return H3C_OK;
}

int sock_recv(struct sock *sock, int len, struct nng_iov *iov, void *arg)

{
    struct packet *packet = packet_new();
    if (!packet) return H3C_ENOMEM;

    if (!(packet->msg = mrecv(sock->stream, len, iov)))
    {
        packet_del(packet);
        return H3C_ENOMEM;
    }
    packet->arg = arg;

    cco_queue_put(&sock->recv_queue, &packet->node);
    return H3C_OK;
}

int sock_send_flat(struct sock *sock, size_t len, void const *buf, void *arg)
{
    struct nng_iov iov = {.iov_len = len, .iov_buf = (void *)buf};
    return sock_send(sock, 1, &iov, arg);
}

int sock_recv_flat(struct sock *sock, size_t len, void *buf, void *arg)
{
    struct nng_iov iov = {.iov_len = len, .iov_buf = buf};
    return sock_recv(sock, 1, &iov, arg);
}

static struct packet *wait_next(struct cco_queue *queue);

struct packet *sock_wait_send(struct sock *sock)
{
    return wait_next(&sock->send_queue);
}

struct packet *sock_wait_recv(struct sock *sock)
{
    return wait_next(&sock->recv_queue);
}

void sock_close(struct sock *sock)
{
    if (sock->stream)
    {
        nng_stream_close(sock->stream);
        nng_stream_free(sock->stream);
    }
    if (sock->aio) nng_aio_free(sock->aio);
    sock->stream = NULL;
    sock->aio = NULL;
}

void sock_del(struct sock *sock)
{
    if (!sock) return;
    sock_close(sock);
    free(sock);
}

static struct packet *wait_next(struct cco_queue *queue)
{
    if (cco_queue_empty(queue)) return NULL;

    void *ptr = cco_queue_pop(queue);
    struct packet *packet = cco_of(ptr, struct packet, node);
    packet->result = nnge(mwait(packet->msg));

    return packet;
}
