#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>

void *xmalloc(size_t size)
{
    void *ptr;
    ptr = malloc(size);
    if (!ptr) {
        err(1, "FATAL: allocation failed");
    }

    memset(ptr, 0, size);

    return ptr;
}

void *xcalloc(size_t nmemb, size_t size)
{
    void *ptr;
    ptr = calloc(nmemb, size);
    if (!ptr) {
        err(1, "FATAL: allocation failed");
    }

    return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
    void *tmp;
    tmp = realloc(ptr, size);
    if (tmp == NULL) {
        err(1, "FATAL: allocation failed");
    }

    return tmp;
}

char *xstrdup(const char *s)
{
    char *dup;
    dup = strdup(s);
    if (!dup) {
        err(1, "FATAL: allocation failed");
    }

    return dup;
}

int mkdirs(const char *name, mode_t mode)
{
    if (strcmp("/", name) == 0 || access(name, F_OK) == 0) {
        return 0;
    } else {
        int stat;
        char *dir;

        dir = xstrdup(name);
        dir = dirname(dir);

        stat = mkdirs(dir, mode);

        free(dir);
        dir = NULL;

        if (stat == 0 && mkdir(name, mode) < 0) {
            return -1;
        }

        return stat;
    }
}
