#ifndef TASK_H
#define TASK_H

struct task;
struct sock;
struct request;
struct answer;

struct task *task_new(void);
void task_open(struct task *, struct sock *);
int task_recv(struct task *t);
void task_close(struct task *);
void task_del(struct task *);

struct request *task_request(struct task *);
struct answer *task_answer(struct task *);

#endif
