#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wordexp.h>
#include <readline/readline.h>
#include <unistd.h>

static int cmd(int argc, char **argv)
{
    printf("argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("[%d]: '%s'\n", i, argv[i]);

        //if (strncmp(argv[i], "test", strlen("test")) == 0) {
        //    clitest(argc, argv);
        //}
    }

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
    wordexp_t p;
    errno = 0;
    while (readargv(prompt, &p) == 0) {
        cmd(p.we_wordc, p.we_wordv);
        wordfree(&p);
        errno = 0;
    }
}

static int cli(int argc, char **argv)
{
    int aflag, bflag, index, c;
    char *cvalue, *svalue;

    aflag = 0;
    bflag = 0;
    cvalue = NULL;
    svalue = NULL;

    opterr = 0;

    while((c = getopt(argc, argv, ":abc:s::")) != -1) {
        switch (c)
        {
         case 'a':
            aflag = 1;
            break;
        case 'b':
            bflag = 1;
            break;
        case 'c':
            cvalue = optarg;
            cmdloop("sk -> cook -> ");
            break;
        case 's':
            svalue = optarg;
            cmdloop("sk -> scale -> ");
            break;
        case '?':
            if (optopt == 'c') {
                //fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                printf ("c flag requires an argument");
                return cli(argc, argv);
            } else if (isprint (optopt)) {
                printf ("Options: abcs");
                return cli(argc, argv);
            }
        default:
            //cli(argc, argv);
            abort ();
        }
    }

    //printf("aflag = %d, bflag = %d, cvalue = %s\n, svalue= %s\n",
          //aflag, bflag, cvalue, svalue);

    //for (index = optind; index < argc; index++) {
    //    printf ("Non-option argument %s\n", argv[index]);
    //}

    //if (strncmp(argv[1], "exit", strlen("exit")) != 0) {
    //    return cli(argc, argv);
    //}

    return 0;
}


int main(int argc, char *argv[])
{
    //cmdloop("[sk] -> ");
    cli(argc, argv);
    return 0;
}
