#include "helper.h"
#include <stdlib.h>

void __check_code(int code, char const *file, int line)
{
    if (!code) return;
    fprintf(stderr, "%s:%d: %s\n", file, line, h3c_decode(code));
    exit(EXIT_FAILURE);
}

void __fail(char const *file, int line)
{
    fprintf(stderr, "%s:%d failure\n", file, line);
    exit(EXIT_FAILURE);
}

static inline double fabs(double x) { return x < 0 ? -x : x; }

bool is_close(double a, double b) { return fabs(a - b) < 1e-7; }

char *append_char(size_t n, char *dst, char c)
{
    char *t = realloc(dst, n + 1);
    if (!t) exit(EXIT_FAILURE);
    t[n] = c;
    return t;
}

struct query const ross[3] = {
    [GOOD] = {.name = "tr|Q949S7|Q949S7_ARATH",
              .desc = "NAD(P)-binding Rossmann-fold superfamily protein "
                      "OS=Arabidopsis thaliana OX=3702 GN=At5g15910 PE=1 SV=1",
              .seq =
                  "MLRSLIWKRSQAYSSVVTMSSISQRGNERLLSEVAGSHSRDNKILVLGGNGYVGSHICKE"
                  "ALRQGFSVSSLSRSGRSSLHDSWVDDVTWHQGDLLSPDSLKPALEGITSVISCVGGFGSN"
                  "SQMVRINGTANINAVKAAAEQGVKRFVYISAADFGVINNLIRGYFEGKRATEAEILDKFG"
                  "NRGSVLRPGFIHGTRQVGSIKLPLSLIGAPLEMVLKLLPKEVTKIPVIGPLLIPPVNVKS"
                  "VAATAVKAAVDPEFASGVIDVYRILQHGH"},
    [BAD] = {.name = ">tr|Q|Q_ARATH",
             .desc = "NAD(P)-binding Rossmann-fold",
             .seq = "MLRSLIWRSQAYSSVVTMSSISQRGNERLLSEVAGSHSRDNKILVLGNGYVGSHICKE"
                    "SQMVRINGTANINAVKAAAEQGVKRFVYISADFGVINNLIRGYFEGKRATEAEILDKF"
                    "NRGSVLRPGFIHGTRQVGSIKLPLSLIGAPLMVLKLLPKEVTKIPVIGPLLIPPVNVS"
                    "TAATAVKAAVDPEFASGVIDVYRILQHGH"},
    [CORRUPTED] = {.name = ">a", .desc = "", .seq = "__FF"}};
