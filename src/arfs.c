#include "arfs.h"
#include "util.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <assert.h>
#include <archive.h>

struct arfs {
    char *path;
    struct archive *aw;
    struct archive *ar;
};

struct ardir {
};

struct arfile {
};

/* similar to err(3), but checks archive_errno */
static void erra(struct archive *a, int eval, const char *fmt, ...)
{
    int arrsv, errsv;
    va_list ap;

    errsv = errno;
    arrsv = archive_errno(a);

    if (program_invocation_short_name) {
        fprintf(stderr, "%s: ", program_invocation_short_name);
    }

    va_start(ap, fmt);
    fprintf(stderr, fmt, ap);
    va_end(ap);

    if (arrsv) {
        fprintf(stderr, ": %s", archive_error_string(a));
    } else if (errsv) {
        fprintf(stderr, ": %s", strerror(errsv));
    }

    fprintf(stderr, "\n");

    exit(eval);
}

/* get the writable version of the database archive */
static struct archive *writear(arfs_t *fs)
{
    if (fs->aw) {
        return fs->aw;
    }

    fs->aw = archive_write_new();
    if (!fs->aw) {
        errx(1, "error creating new database archive");
    }

    if (archive_write_set_format_pax_restricted(fs->aw) != ARCHIVE_OK) {
        erra(fs->aw, 1, "error setting archive options");
    }

    if (archive_write_add_filter_gzip(fs->aw) != ARCHIVE_OK) {
        erra(fs->aw, 1, "error setting archive options");
    }

    if (archive_write_open_filename(fs->aw, fs->path) != ARCHIVE_OK) {
        erra(fs->aw, 1, "error setting archive options");
    }

    return fs->aw;
}

/* get the readable version of the database archive */
static struct archive *readar(arfs_t *fs)
{
    if (fs->ar) {
        return fs->ar;
    }

    fs->ar = archive_read_new();
    if (!fs->ar) {
        errx(1, "error creating new database archive");
    }

    if (archive_read_support_format_tar(fs->ar) != ARCHIVE_OK) {
        erra(fs->ar, 1, "error setting archive options");
    }

    if (archive_read_support_filter_gzip(fs->ar) != ARCHIVE_OK) {
        erra(fs->ar, 1, "error setting archive options");
    }

    if (archive_read_open_filename(fs->ar, fs->path, 10240) != ARCHIVE_OK) {
        erra(fs->ar, 1, "error setting archive options");
    }

    return fs->ar;
}

arfs_t *arfs_open(const char *path, int flags)
{
    arfs_t *fs;

    if (!path || !flags || !(flags & FS_RDWR)) {
        errno = EINVAL;
        return NULL;
    }

    if (!(flags & FS_CREATE)) {
        int mode;

        mode = 0;
        if (flags & FS_RD) {
            mode |= R_OK;
        }
        if (flags & FS_WR) {
            mode |= W_OK;
        }

        assert(mode != 0);
        if (access(path, mode) != 0) {
            return NULL;
        }
    }

    fs = (arfs_t *)xmalloc(sizeof(arfs_t));
    fs->path = xstrdup(path);

    return fs;
}

ardir_t *arfs_root()
{
    return NULL;
}

ardir_t *arfs_mkdir(ardir_t *dir, const char *name)
{
    return NULL;
}

arfile_t *arfs_mkfile(ardir_t *dir, const char *name, const uint8_t *buf, size_t len)
{
    return NULL;
}

void arfs_close(arfs_t *fs)
{
    if (fs->aw) {
        if (archive_write_close(fs->aw) != ARCHIVE_OK) {
            erra(fs->aw, 1, "error writing file data");
        }

        if (archive_write_free(fs->aw) != ARCHIVE_OK) {
            errx(1, "error writing file data");
        }
    }

    if (fs->ar) {
        if (archive_read_close(fs->ar) != ARCHIVE_OK) {
            erra(fs->ar, 1, "error reading file data");
        }

        if (archive_read_free(fs->ar) != ARCHIVE_OK) {
            errx(1, "error reading file data");
        }
    }

    free(fs->path);
    fs->path = NULL;

    free(fs);
}
