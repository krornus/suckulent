#include "arfs.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <wordexp.h>
#include <readline/readline.h>

static int cmd(int argc, char **argv)
{
    printf("argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("[%d]: '%s'\n", i, argv[i]);
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

int main(int argc, char *argv[])
{
    arfs_t *fs;
    fs = arfs_open("./test.db", AFS_O_WR | AFS_O_CREATE);

    if (!arfs_mkfile(arfs_root(fs), "./a", (uint8_t *)"aaaa", 4))
        err(1, "failed to make new file");

    if (!arfs_mkfile(arfs_root(fs), "./b", (uint8_t *)"bbbb", 4))
        err(1, "failed to make new file");

    if (!arfs_mkdir(arfs_root(fs), "./b"))
        err(1, "failed to make new dir");

    arfs_close(fs);

    return 0;
}
