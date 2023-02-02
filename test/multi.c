#include "fs.h"
#include "h3c/h3c.h"
#include "helper.h"
#include "hope.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 51371
static char const cmd[] = "--hmmdb 1 --acc --cut_ga --hmmdb_ranges 0..4";

static void test_multi(void);

int main(void)
{
    atexit(h3c_fini);
    test_multi();
    return hope_status();
}

static void test_multi(void)
{
    struct h3c_dialer *d = h3c_dialer_new("127.0.0.1", PORT);
    notnull(d);
    eq(h3c_dialer_dial(d, h3c_deadline(1000)), 0);

    struct h3c_stream *s = h3c_dialer_stream(d);

    struct h3c_result *result = h3c_result_new();
    notnull(result);

    long deadline = h3c_deadline(1000 * 5);
    for (size_t i = 0; i < array_size(ross); ++i)
    {
        eq(h3c_stream_put(s, cmd, ross[i].name, ross[i].seq, deadline), 0);
    }

    for (size_t i = 0; i < array_size(ross); ++i)
    {
        h3c_stream_wait(s);
        eq(h3c_stream_pop(s, result), 0);
        eq(h3c_result_errnum(result), 0);
        eq(h3c_result_nhits(result), ross[i].expect.nhits);
        if (h3c_result_nhits(result) > 0)
        {
            double lev = h3c_result_hit_evalue_ln(result, 0);
            close(lev, ross[i].expect.ln_evalue);
        }
    }

    h3c_result_del(result);
    h3c_stream_del(s);
    h3c_dialer_del(d);
}
