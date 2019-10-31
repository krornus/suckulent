#include <err.h>
#include <stdlib.h>

void *xmalloc(size_t size)
{
    void *ptr;
    ptr = malloc(size);
    if (!ptr) {
        err(1, "FATAL: allocation failed");
    }

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
}
