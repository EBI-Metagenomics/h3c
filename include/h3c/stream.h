#ifndef H3C_STREAM_H
#define H3C_STREAM_H

struct h3c_stream;
struct h3c_result;

int h3c_stream_put(struct h3c_stream *, char const *args, char const *seq,
                   long deadline);
void h3c_stream_wait(struct h3c_stream *);
int h3c_stream_pop(struct h3c_stream *, struct h3c_result *);
void h3c_stream_del(struct h3c_stream *);

#endif
