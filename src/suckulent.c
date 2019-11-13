#include "arfs.h"
#include "util.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <wordexp.h>
#include <readline/readline.h>

static int arfs_cmd(arfile_t **cwd, int argc, char **argv)
{
    if (argc == 0) {
        return 0;
    }

    if (strcmp(argv[0], "cd") == 0) {
        if (argc != 2) {
            errno = EINVAL;
            warn("cd");
            return 1;
        } else {
            arfile_t *dir;
            dir = arfs_dir(*cwd, argv[1]);
            if (!dir) {
                errno = ENOENT;
                warn("cd");
                return 1;
            } else {
                *cwd = dir;
            }
        }
    } else if (strcmp(argv[0], "mkdir") == 0) {
        if (argc != 2) {
            errno = EINVAL;
            warn("mkdir");
            return 1;
        } else {
            arfile_t *dir;
            errno = 0;
            dir = arfs_mkdir(*cwd, argv[1]);
            if (!dir) {
                if (!errno) {
                    errno = EINVAL;
                }
                warn("mkdir");
                return 1;
            }
        }
    } else if (strcmp(argv[0], "mkfile") == 0) {
        if (argc != 3) {
            errno = EINVAL;
            warn("mkfile");
            return 1;
        } else {
            arfile_t *file;
            errno = 0;
            file = arfs_mkfile(*cwd, argv[1], (uint8_t *)argv[2], strlen(argv[2]));
            if (!file) {
                if (!errno) {
                    errno = EINVAL;
                }
                warn("mkfile");
                return 1;
            }
        }
    } else {
        if (errno) {
            warn("unrecognized command");
        } else {
            warnx("unrecognized command");
            errno = 0;
        }
        return 1;
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

static void cmdloop()
{
    char *prompt;
    wordexp_t p;
    arfs_t *fs;
    arfile_t *cwd;

    fs = arfs_open("./test.db", AFS_O_WR | AFS_O_CREATE);
    cwd = arfs_root(fs);

    prompt = xstrdup("[sk] - arfs : /\n--> ");

    errno = 0;
    while (readargv(prompt, &p) == 0) {
        arfs_cmd(&cwd, p.we_wordc, p.we_wordv);
        wordfree(&p);
        free(prompt);
        prompt = NULL;
        if (asprintf(&prompt, "[sk] - arfs : %s\n--> ", xarfs_path(cwd, NULL)) == -1) {
            err(1, "prompt allocation failed");
        }

        errno = 0;
    }
    arfs_close(fs);
}

int main(int argc, char *argv[])
{
    cmdloop();
    return 0;
}
