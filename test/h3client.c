#include "h3client/h3client.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

struct h3request request = {0};
struct h3answer answer = {0};

#define DONE printf("done.\n")
#define FAIL                                                                   \
    do                                                                         \
    {                                                                          \
        printf("failed!\n");                                                   \
        return 1;                                                              \
    } while (0)

int main(void)
{
    // conn.go = esl_getopts_Create(searchOpts);
    // esl_getopts_Destroy(conn.go);

    printf("Connecting to master... ");
    if (h3conn_open("192.168.1.10", 51371))
        DONE;
    else
        FAIL;

    // h3request_setup(&request, "--hmmdb 1 --hmmdb_idx 12492 --acc --cut_ga");

    printf("Initing request... ");
    if (h3request_init(&request))
        DONE;
    else
        FAIL;

    printf("Initing answer... ");
    if (h3answer_init(&answer))
        DONE;
    else
        FAIL;

    h3request_setup_args(&request, "--hmmdb 1 --acc --cut_ga");

    printf("Openning fasta file... ");
    FILE *fasta_file = fopen(ASSETS "/ross.fasta", "r");
    if (fasta_file)
        DONE;
    else
        FAIL;

    printf("Loading fasta file... ");
    if (h3request_setup_fasta(&request, fasta_file))
        DONE;
    else
        FAIL;
    fclose(fasta_file);

    int rc = !h3conn_call(&request, &answer);

    h3request_cleanup(&request);
    h3answer_cleanup(&answer);
    h3conn_close();

    return rc;
}
