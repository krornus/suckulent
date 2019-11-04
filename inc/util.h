#ifndef util_h_INCLUDED
#define util_h_INCLUDED

#include <sys/types.h>

void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *s);

int mkdirs(const char *name, mode_t mode);

#endif // util_h_INCLUDED

