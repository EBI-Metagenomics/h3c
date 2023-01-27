#ifndef H3C_RESULT_H
#define H3C_RESULT_H

#include "h3c/export.h"
#include <stdbool.h>
#include <stdio.h>

struct h3c_result;

H3C_API struct h3c_result *h3c_result_new(void);
H3C_API void h3c_result_del(struct h3c_result const *);

H3C_API int h3c_result_pack(struct h3c_result const *, FILE *file);
H3C_API int h3c_result_unpack(struct h3c_result *, FILE *file);

H3C_API void h3c_result_print_targets(struct h3c_result const *, FILE *file);
H3C_API void h3c_result_print_domains(struct h3c_result const *, FILE *file);

H3C_API void h3c_result_print_targets_table(struct h3c_result const *,
                                            FILE *file);
H3C_API void h3c_result_print_domains_table(struct h3c_result const *,
                                            FILE *file);

H3C_API unsigned h3c_result_nhits(struct h3c_result const *);
H3C_API char const *h3c_result_hit_name(struct h3c_result const *,
                                        unsigned idx);
H3C_API char const *h3c_result_hit_acc(struct h3c_result const *, unsigned idx);
H3C_API double h3c_result_hit_evalue_ln(struct h3c_result const *,
                                        unsigned idx);

#endif
