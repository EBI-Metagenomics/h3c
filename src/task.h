#ifndef TASK_H
#define TASK_H

#include "sock.h"

struct request;
struct answer;

struct task
{
    struct request *request;
    struct answer *answer;
    struct sock sock;
};

void task_init(struct task *);
int task_open(struct task *);
void task_close(struct task *);

#endif
