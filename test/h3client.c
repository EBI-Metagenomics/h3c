#include "h3client/h3client.h"
#include "file_hash.h"

#ifdef _POSIX_C_SOURCE
#if _POSIX_C_SOURCE < 200112L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#else
#define _POSIX_C_SOURCE 200112L
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static char const *h3master_address(void)
{
    char const *addr = getenv("H3CLIENT_MASTER_ADDRESS");
    return addr ? addr : "127.0.0.1";
}

static struct h3c_result *result = 0;

#define FAIL(msg)                                                              \
    do                                                                         \
    {                                                                          \
        exit_status = 1;                                                       \
        printf(msg);                                                           \
        putchar('\n');                                                         \
        goto cleanup;                                                          \
    } while (0);

int main(void)
{
    int exit_status = 0;
    FILE *fasta_file = 0;
    FILE *result_file = 0;

    if (h3c_open(h3master_address(), 51371)) FAIL("Failed to h3c_open!");

    fasta_file = fopen(ASSETS "/ross.fasta", "r");
    if (!fasta_file) FAIL("Failed to fopen!");

    if (!(result = h3c_result_new())) FAIL("Failed to h3c_result_new!")

    if (h3c_call("--hmmdb 1 --acc --cut_ga", fasta_file, result))
        FAIL("Failed to h3c_call!");

    fclose(fasta_file);

    result_file = fopen(TMPDIR "/h3result.msgpack", "wb");
    if (!result_file) FAIL("Failed to fopen!");
    if (h3c_result_pack(result, result_file))
        FAIL("Failed to h3c_result_pack!");
    fclose(result_file);

    result_file = fopen(TMPDIR "/h3result.msgpack", "rb");
    if (!result_file) FAIL("Failed to fopen!");
    int64_t hash = 0;
    if (!file_hash(result_file, &hash)) FAIL("Failed to file_hash!");
    printf("hash: %lld\n", hash);
    if (hash != 6744319896512311420LL)
        FAIL("Wrong file hash for h3result.msgpack!");
    fclose(result_file);

    h3c_pack_print();

    if (h3c_close()) FAIL("Failed to h3c_close!");
    h3c_result_del(result);

cleanup:
    return exit_status;
}
