#include "task.h"
#include "answer.h"
#include "atomic.h"
#include "cco.h"
#include "h3c/code.h"
#include "msg.h"
#include <nng/nng.h>
#include <stdlib.h>

struct task
{
    struct nng_stream *stream;
    struct cco_queue queue;
};

struct task *task_new(struct nng_stream *stream)
{
    struct task *task = malloc(sizeof(*task));
    if (!task) return NULL;
    task->stream = stream;
    cco_queue_init(&task->queue);
    return task;
}

int task_put(struct task *t, char const *args, char const *seq, long deadline)
{
    struct msg *msg = msg_new(t->stream);
    if (!msg) return H3C_ENOMEM;

    int rc = msg_start(msg, args, seq, deadline);
    if (rc)
    {
        msg_del(msg);
        return rc;
    }

    task_wait(t);
    cco_queue_put(&t->queue, &msg->node);

    return H3C_OK;
}

void task_wait(struct task *t)
{
    if (cco_queue_empty(&t->queue)) return;
    struct msg *msg = cco_of(cco_queue_pop(&t->queue), struct msg, node);
    msg_wait(msg);
    cco_queue_put_first(&t->queue, &msg->node);
}

int task_pop(struct task *t, struct h3c_result *r)
{
    struct msg *msg = cco_of(cco_queue_pop(&t->queue), struct msg, node);
    int rc = answer_copy(msg_answer(msg), r);
    msg_del(msg);
    return rc;
}

void task_del(struct task *t)
{
    while (!cco_queue_empty(&t->queue))
    {
        task_wait(t);
        struct msg *msg = cco_of(cco_queue_pop(&t->queue), struct msg, node);
        msg_del(msg);
    }
}
