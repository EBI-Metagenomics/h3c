#ifndef H3C_STREAM_H
#define H3C_STREAM_H

#include "h3c/export.h"

struct h3c_stream;
struct h3c_result;

H3C_API int h3c_stream_put(struct h3c_stream *, char const *args,
                           char const *name, char const *seq, long deadline);
H3C_API void h3c_stream_wait(struct h3c_stream *);
H3C_API int h3c_stream_pop(struct h3c_stream *, struct h3c_result *);
H3C_API void h3c_stream_del(struct h3c_stream *);

#endif
