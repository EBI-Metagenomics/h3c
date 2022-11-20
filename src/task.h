#ifndef TASK_H
#define TASK_H

struct h3c_result;
struct nng_stream;
struct task;

struct task *task_new(struct nng_stream *stream);
int task_put(struct task *, char const *args, char const *seq, long deadline);
void task_wait(struct task *);
int task_pop(struct task *, struct h3c_result *);
void task_del(struct task *);

#endif
