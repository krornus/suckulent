#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wordexp.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

static void other(int aflag, int bflag)
{
    int sum;

    printf("%d %d\n", aflag, bflag);

    sum = 0;

    sum += aflag;
    sum += bflag;

    printf("Other: %d\n", sum);
}

static void test(int aflag, int bflag)
{
    int sum;

    printf("%d %d\n", aflag, bflag);

    sum = 0;

    sum += aflag;
    sum += bflag;

    printf("Test: %d\n", sum);
}

static int cli_start(int argc, char **argv, int sub)
{
    size_t i;

    int aflag, bflag, c;

    const char *commands[2];

    commands[0] = "test";
    commands[1] = "other";

    aflag = 0;
    bflag = 0;

    opterr = 0;
    optind = sub + 1; //Either start at 1 or 2 to only parse flag options

    while((c = getopt(argc, argv, "ab")) != -1) {
        switch (c)
        {
        case 'a':
            aflag = 1;
            break;
        case 'b':
            bflag = 1;
            break;
        case '?':
            printf("No option '-%c'.\n", optopt);
        }
    }

    for (i = 0; i < 2; i++) {
        if (strncmp(argv[sub], commands[i], strlen(commands[i])) == 0) {
            switch (i)
            {
                case 0:
                    test(aflag, bflag);
                    return 0;
                case 1:
                    other(aflag, bflag);
                    return 0;
            }
        }
    }

    printf("Command '%s' not found.\n", argv[0]);

    return 0;
}

static int readargv(char *prompt, wordexp_t *p)
{
    char *line;

    line = readline(prompt);
    if (line == NULL) {
        return 1;
    }

    if (wordexp(line, p, 0) != 0) {
        err(1, "FATAL: wordexp");
    }

    return 0;
}

static void cmdloop(char *prompt)
{
    using_history(); /*initialize history*/
    register HISTORY_STATE *hist;
    register HIST_ENTRY **hist_list;

    int i;

    wordexp_t p;
    errno = 0;
    while (readargv(prompt, &p) == 0) {
        // cmd(p.we_wordc, p.we_wordv);
        cli_start(p.we_wordc, p.we_wordv, 0);
        add_history(p.we_wordv[0]);
        wordfree(&p);
        errno = 0;
    }

    hist = history_get_history_state();
    hist_list = history_list();

    printf ("\nSession History\n\n");

    for (i = 0; i < hist->length; i++) { /* output history list */
        printf (" %8s  %s\n", hist[i]->line, hist_list[i]->timestamp);
        free_history_entry (hist_list[i]);     /* free allocated entries */
    }

    putchar('\n');

    free(hist_list);  /* free HIST_ENTRY list */
    free(hist);  /* free HISTORY_STATE   */
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        cmdloop("[sk] -> ");
    } else {
        cli_start(argc, argv, 1);
        cmdloop("[sk] -> ");
    }

    return 0;
}
