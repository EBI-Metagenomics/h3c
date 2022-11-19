#include "task.h"
#include "answer.h"
#include "h3c/code.h"
#include "hmmd/hmmd.h"
#include "request.h"
#include <stddef.h>

void task_init(struct task *t)
{
    t->request = NULL;
    t->answer = NULL;
    sock_init(&t->sock);
}

int task_open(struct task *t)
{
    int rc = H3C_OK;

    if (!(t->request = request_new()))
    {
        rc = H3C_ENOMEM;
        goto cleanup;
    }

    if (!(t->answer = answer_new()))
    {
        rc = H3C_ENOMEM;
        goto cleanup;
    }

    if ((rc = sock_open(&t->sock))) goto cleanup;

    return rc;

cleanup:
    task_close(t);
    return rc;
}

void task_close(struct task *t)
{
    sock_close(&t->sock);
    if (t->answer) answer_del(t->answer);
    if (t->request) request_del(t->request);
    t->answer = NULL;
    t->request = NULL;
}

int task_recv(struct task *t)
{
    int rc = H3C_OK;

    void *data = answer_status_data(t->answer);
    size_t size = answer_status_size();
    if ((rc = sock_recv(&t->sock, size, data))) return rc;

    struct hmmd_status const *status = answer_status_parse(t->answer);

    size = status->msg_size;
    if ((rc = answer_setup_size(t->answer, size))) return rc;

    data = answer_data(t->answer);
    if ((rc = sock_recv(&t->sock, size, data))) return rc;

    if (!status->status)
    {
        if ((rc = answer_parse(t->answer))) return rc;
        // if ((rc = answer_copy(s->answer, result))) return rc;
    }

    return rc;
}
