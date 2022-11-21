#include "h3c/stream.h"
#include "answer.h"
#include "cco.h"
#include "h3c/code.h"
#include "msg.h"
#include "stream.h"
#include <nng/nng.h>
#include <stdlib.h>

struct h3c_stream
{
    struct nng_stream *stream;
    struct cco_queue queue;
};

struct h3c_stream *h3c_stream_new(struct nng_stream *stream)
{
    if (!stream) return NULL;

    struct h3c_stream *task = malloc(sizeof(*task));
    if (!task) return NULL;

    task->stream = stream;
    cco_queue_init(&task->queue);
    return task;
}

int h3c_stream_put(struct h3c_stream *t, char const *args, char const *seq,
                   long deadline)
{
    struct msg *msg = msg_new(t->stream);
    if (!msg) return H3C_ENOMEM;

    h3c_stream_wait(t);
    int rc = msg_start(msg, args, seq, deadline);
    if (rc)
    {
        msg_del(msg);
        return rc;
    }

    cco_queue_put(&t->queue, &msg->node);

    return H3C_OK;
}

void h3c_stream_wait(struct h3c_stream *t)
{
    if (cco_queue_empty(&t->queue)) return;
    struct msg *msg = cco_of(cco_queue_pop(&t->queue), struct msg, node);
    msg_wait(msg);
    cco_queue_put_first(&t->queue, &msg->node);
}

int h3c_stream_pop(struct h3c_stream *t, struct h3c_result *r)
{
    struct msg *msg = cco_of(cco_queue_pop(&t->queue), struct msg, node);
    int rc = answer_copy(msg_answer(msg), r);
    msg_del(msg);
    return rc;
}

void h3c_stream_del(struct h3c_stream *t)
{
    if (!t) return;
    while (!cco_queue_empty(&t->queue))
    {
        h3c_stream_wait(t);
        struct msg *msg = cco_of(cco_queue_pop(&t->queue), struct msg, node);
        msg_del(msg);
    }
    nng_stream_close(t->stream);
    nng_stream_free(t->stream);
    free(t);
}
