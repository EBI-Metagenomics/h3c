#include "argless.h"
#include "h3c/result.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static struct h3c_result *result = NULL;
static FILE *file = NULL;

static bool init(char const *filepath);
static void cleanup(void);
static bool err(char const *msg);

static struct argl_option const options[] = {
    {"targets", 't', ARGL_FLAG(), "Print targets"},
    {"domains", 'd', ARGL_FLAG(), "Print domains"},
    {"targets-table", 'T', ARGL_FLAG(), "Print targets table"},
    {"domains-table", 'D', ARGL_FLAG(), "Print domains table"},
    ARGL_DEFAULT,
    ARGL_END,
};

static struct argl argl = {.options = options,
                           .args_doc = "INPUT_FILE",
                           .doc = "Read h3 result files.",
                           .version = "0.3.0"};

int main(int argc, char *argv[])
{
    argl_parse(&argl, argc, argv);
    if (argl_nargs(&argl) != 1) argl_usage(&argl);

    if (!init(argl_args(&argl)[0])) return EXIT_FAILURE;

    if (h3c_result_unpack(result, file))
    {
        err("failed to parse result");
        cleanup();
        return EXIT_FAILURE;
    }

    if (!strcmp(argl_get(&argl, "targets"), "true"))
        h3c_result_print_targets(result, stdout);

    if (!strcmp(argl_get(&argl, "domains"), "true"))
        h3c_result_print_domains(result, stdout);

    if (!strcmp(argl_get(&argl, "targets-table"), "true"))
        h3c_result_print_targets_table(result, stdout);

    if (!strcmp(argl_get(&argl, "domains-table"), "true"))
        h3c_result_print_domains_table(result, stdout);

    cleanup();
    return EXIT_SUCCESS;
}

static bool init(char const *filepath)
{
    if (!(result = h3c_result_new())) return err("failed to allocate result");
    if (!(file = fopen(filepath, "rb")))
    {
        cleanup();
        return err("failed to open file");
    }
    return true;
}

static void cleanup(void)
{
    if (result) h3c_result_del(result);
    result = NULL;

    if (file) fclose(file);
    file = NULL;
}

static bool err(char const *msg)
{
    fputs(msg, stderr);
    fputs("\n", stderr);
    return false;
}
