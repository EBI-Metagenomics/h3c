#include "task.h"
#include "answer.h"
#include "h3c/code.h"
#include "hmmd/hmmd.h"
#include "request.h"
#include "sock.h"
#include <stddef.h>
#include <stdlib.h>

struct task
{
    struct request *request;
    struct answer *answer;
    struct sock *sock;
};

struct task *task_new(void)
{
    struct task *t = malloc(sizeof(*t));
    if (!t) return NULL;
    if (!(t->request = request_new())) goto cleanup;
    if (!(t->answer = answer_new())) goto cleanup;
    t->sock = NULL;
    return t;

cleanup:
    task_del(t);
    return NULL;
}

void task_open(struct task *t, struct sock *sock) { t->sock = sock; }

int task_recv(struct task *t)
{
    int rc = H3C_OK;

    void *data = answer_status_data(t->answer);
    size_t size = answer_status_size();
    if ((rc = sock_recv(t->sock, size, data, NULL))) return rc;

    struct hmmd_status const *status = answer_status_parse(t->answer);

    size = status->msg_size;
    if ((rc = answer_setup_size(t->answer, size))) return rc;

    data = answer_data(t->answer);
    if ((rc = sock_recv(t->sock, size, data, NULL))) return rc;

    if (!status->status)
    {
        if ((rc = answer_parse(t->answer))) return rc;
        // if ((rc = answer_copy(s->answer, result))) return rc;
    }

    return rc;
}

void task_close(struct task *t)
{
    sock_close(t->sock);
    if (t->answer) answer_del(t->answer);
    if (t->request) request_del(t->request);
    t->answer = NULL;
    t->request = NULL;
}

void task_del(struct task *t)
{
    if (!t) return;
    task_close(t);
    if (t->sock) sock_del(t->sock);
    free(t);
}

struct request *task_request(struct task *t) { return t->request; }

struct answer *task_answer(struct task *t) { return t->answer; }
