#ifndef util_h_INCLUDED
#define util_h_INCLUDED

#include <sys/types.h>

#undef GET_PROGRAM_NAME
#ifdef __GLIBC__
#   define GET_PROGRAM_NAME() program_invocation_short_name
#else /* *BSD and OS X */
#   include <stdlib.h>
#   define GET_PROGRAM_NAME() getprogname()
#endif

void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *s);

int mkdirs(const char *name, mode_t mode);

#endif // util_h_INCLUDED
