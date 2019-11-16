#include "arfs.h"
#include "util.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <libgen.h>
#include <err.h>
#include <errno.h>
#include <assert.h>
#include <archive.h>
#include <archive_entry.h>

#define ARX(A, X) do {\
    if (X != ARCHIVE_OK) {\
        erra(A, 1, "archive error");\
    }\
} while (0)

struct arfs {
    char *path;
    int mode;
    struct archive *aw;
    struct archive *ar;
    arfile_t *root;
};

enum AFSFileType {
    AFS_FILE,
    AFS_LINK,
    AFS_DIR,
};

struct arfile {
    enum AFSFileType ty;
    struct archive_entry *ent;
    const char *name;
    arfs_t *fs;
    arfile_t *next;
    /* directory specific */
    arfile_t *parent;
    arfile_t *entries;
};

/* similar to err(3), but checks archive_errno */
static void erra(struct archive *a, int eval, const char *fmt, ...)
{
    int arrsv, errsv;
    va_list ap;

    errsv = errno;
    arrsv = archive_errno(a);

    if (GET_PROGRAM_NAME()) {
        fprintf(stderr, "%s: ", GET_PROGRAM_NAME());
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
    if (!(fs->mode & AFS_O_WR)) {
        errno = EPERM;
        err(1, "cannot get archive");
    }

    if (fs->aw) {
        return fs->aw;
    }

    fs->aw = archive_write_new();
    if (!fs->aw) {
        errx(1, "error creating new database archive");
    }

    ARX(fs->aw, archive_write_add_filter_gzip(fs->aw));
    ARX(fs->aw, archive_write_set_format_pax_restricted(fs->aw));
    ARX(fs->aw, archive_write_open_filename(fs->aw, fs->path));

    return fs->aw;
}

/* get the readable version of the database archive */
static struct archive *readar(arfs_t *fs)
{
    if (!(fs->mode & AFS_O_RD)) {
        errno = EPERM;
        err(1, "cannot get archive");
    }

    if (fs->ar) {
        return fs->ar;
    }

    fs->ar = archive_read_new();
    if (!fs->ar) {
        errx(1, "error creating new database archive");
    }

    ARX(fs->ar, archive_read_support_format_tar(fs->ar));
    ARX(fs->ar, archive_read_support_filter_gzip(fs->ar));
    ARX(fs->ar, archive_read_open_filename(fs->ar, fs->path, 10240));

    return fs->ar;
}

static inline char *abasename(const char *name)
{
    char *x, *y, *z;
    x = xstrdup(name);
    y = basename(x);
    z = xstrdup(y);
    free(x);
    return z;
}

static inline char *adirname(const char *name)
{
    char *x, *y, *z;
    x = xstrdup(name);
    y = dirname(x);
    z = xstrdup(y);
    free(x);
    return z;
}

static arfile_t *dir_get(arfile_t *dir, const char *name)
{
    arfile_t *file;
    file = dir->entries;

    if (strcmp(name, "..") == 0) {
        return dir->parent;
    } else if (strcmp(name, ".") == 0) {
        return dir;
    } else if (strcmp(name, "/") == 0) {
        return dir->fs->root;
    }

    while (file) {
        if (strcmp(file->name, name) == 0) {
            return file;
        } else {
            file = file->next;
        }
    }

    errno = ENOENT;
    return NULL;
}

static arfile_t *arfs_get(arfile_t *dir, const char *path)
{
    if (strcmp(path, "..") == 0) {
        return dir->parent;
    } else if (strcmp(path, ".") == 0) {
        return dir;
    } else if (strcmp(path, "/") == 0) {
        return dir->fs->root;
    } else {
        arfile_t *ret;
        char *bn;
        int errsv;

        bn = abasename(path);
        dir = arfs_dirof(dir, path);
        if (dir) {
            ret = dir_get(dir, bn);
        } else {
            ret = NULL;
        }
        errsv = errno;

        free(bn);
        bn = NULL;

        errno = errsv;
        return ret;
    }
}

static arfile_t *arentry(arfs_t *fs, const char *path, int type, arfile_t *parent)
{
    arfile_t *file;

    file = xmalloc(sizeof(arfile_t));

    switch (type) {
    case AE_IFREG:
        file->ty = AFS_FILE;
        break;
    case AE_IFLNK:
        file->ty = AFS_LINK;
        break;
    case AE_IFDIR:
        file->ty = AFS_DIR;
        break;
    default:
        errx(1, "arfs: invalid file type");
    }

    file->name = abasename(path);
    file->fs = fs;
    file->parent = parent;
    file->entries = NULL;
    file->next = NULL;

    return file;
}

static arfile_t *add_file(arfile_t *dir, const char *path, int type)
{
    arfile_t *file;
    file = arentry(dir->fs, path, type, dir);
    if (dir->entries) {
        file->next = dir->entries;
    } else {
        file->next = NULL;
    }

    dir->entries = file;

    return file;
}


static arfile_t *newtree(arfs_t *fs)
{
    arfile_t *root;

    root = xmalloc(sizeof(arfile_t));
    root->ty = AFS_DIR;
    root->name = "/";
    root->fs = fs;
    root->next = NULL;
    root->parent = root;
    root->entries = NULL;

    return root;
}

/* make the filesystem tree - cannot fail */
static arfile_t *fstree(arfs_t *fs, struct archive *a)
{
    int stat;
    const char *path;
    arfile_t *root;
    struct archive_entry *ent;

    root = newtree(fs);

    for (;;) {
        arfile_t *parent;
        char *dn;

        stat = archive_read_next_header(a, &ent);
        if (stat == ARCHIVE_EOF) {
            break;
        } else if (stat != ARCHIVE_OK) {
            err(1, "error reading archive file entry");
        }

        path = archive_entry_pathname(ent);
        if (!path) {
            err(1, "error getting archive path name");
        }

        dn = adirname(path);
        parent = arfs_get(root, dn);
        if (!parent) {
            err(1, "arfs: missing parent directory");
        } else if (parent->ty != AFS_DIR) {
            err(1, "arfs: file type mismatch");
        } else {
            free(dn);
            dn = NULL;
        }

        add_file(parent, path, parent->ty);
    }

    return root;
}



static char *arfs_path_r(arfile_t *file, size_t *len)
{
    if (file->fs->root == file) {
        *len = 1;
        return xstrdup("/");
    } else {
        size_t add;
        char *base, *ret;

        base = arfs_path_r(file->parent, len);
        add = strlen(file->name) + 1;

        ret = (char *)xrealloc(base, *len + add + 1);
        strcat(ret, file->name);
        strcat(ret, "/");

        *len += add;

        return ret;
    }
}

static char *pathjoin(arfile_t *dir, const char *add)
{
    size_t len;
    char *bn, *dn;

    dn = arfs_path(dir, &len);
    bn = abasename(add);

    if (dir_get(dir, bn))
    {
        free(bn);
        bn = NULL;
        free(dn);
        dn = NULL;
        errno = EEXIST;
        return NULL;
    }

    dn = (char *)realloc(dn, len + strlen(bn) + 1);
    strcat(dn, bn);

    free(bn);
    bn = NULL;

    return dn;
}

static arfile_t *mkfile(arfile_t *dir, const char *path, int type, const uint8_t *buf, size_t len)
{
    char *rpath;
    struct archive *a;
    struct archive_entry *ent;

    dir = arfs_dirof(dir, path);
    if (!dir) {
        return NULL;
    }

    rpath = pathjoin(dir, path);
    if (!rpath) {
        return NULL;
    }

    printf("%s\n", rpath);

    a = writear(dir->fs);
    ent = archive_entry_new();
    archive_entry_set_pathname(ent, rpath);
    if (type == AE_IFREG) {
        archive_entry_set_size(ent, len);
    }
    archive_entry_set_filetype(ent, type);

    if (type == AE_IFDIR) {
        archive_entry_set_perm(ent, 0751);
    } else {
        archive_entry_set_perm(ent, 0644);
    }

    ARX(a, archive_write_header(a, ent));

    free(rpath);
    rpath = NULL;

    if (type == AE_IFREG) {
        if (archive_write_data(a, buf, len) != len) {
            erra(a, 1, "failed to write archive data");
        }
        ARX(a, archive_write_finish_entry(a));
    }

    archive_entry_free(ent);

    return add_file(dir, path, type);
}


/* fails on invalid args or missing permissions */
arfs_t *arfs_open(const char *path, int flags)
{
    arfs_t *fs;
    int readable;

    if (!path
        || !flags
        || !(flags & AFS_O_RDWR)
        || (!(flags & AFS_O_WR) && (flags & AFS_O_CREATE)))
    {
        errno = EINVAL;
        return NULL;
    }

    if (!(flags & AFS_O_RD)) {
        readable = 0;
    } else {
        int mode;
        mode = 0;
        if (flags & AFS_O_RD) {
            mode |= R_OK;
        }
        if (flags & AFS_O_WR) {
            mode |= W_OK;
        }
        assert(mode != 0);
        readable = access(path, mode) == 0;
    }

    /* if we want to read, dont want to create, and cant read */
    if (!(flags & AFS_O_CREATE) && (flags & AFS_O_RD) && !readable) {
        errno = ENOENT;
        return NULL;
    }

    fs = (arfs_t *)xmalloc(sizeof(arfs_t));
    fs->path = xstrdup(path);
    fs->mode = flags;

    if (readable) {
        fs->root = fstree(fs, readar(fs));
    } else {
        fs->root = newtree(fs);
    }

    return fs;
}

/* does not fail - dont pass a null fs */
arfile_t *arfs_root(arfs_t *fs)
{
    return fs->root;
}

arfile_t *arfs_dirof(arfile_t *dir, const char *path)
{
    int errsv;
    char *dn;

    if (!dir || !path || dir->ty != AFS_DIR) {
        errno = EINVAL;
        return NULL;
    }

    dn = adirname(path);
    dir = arfs_get(dir, dn);
    errsv = errno;
    free(dn);

    if (dir == NULL) {
        errno = errsv;
        return NULL;
    } else if (dir->ty != AFS_DIR) {
        errno = ENOTDIR;
        return NULL;
    } else {
        return dir;
    }
}

/* fails if arguments are null or file not found or file is dir */
arfile_t *arfs_file(arfile_t *dir, const char *path)
{
    arfile_t *ret;
    if (!dir || !path || dir->ty != AFS_DIR) {
        errno = EINVAL;
        return NULL;
    }

    ret = arfs_get(dir, path);
    if (!ret) {
        errno = ENOENT;
        return NULL;
    } else if (ret->ty == AFS_DIR) {
        errno = EISDIR;
        return NULL;
    } else {
        return ret;
    }
}

/* fails if arguments are null or dir not found or dir is file */
arfile_t *arfs_dir(arfile_t *dir, const char *path)
{
    arfile_t *ret;
    if (!dir || !path || dir->ty != AFS_DIR) {
        errno = EINVAL;
        return NULL;
    }

    ret = arfs_get(dir, path);
    if (!ret) {
        errno = ENOENT;
        return ret;
    } else if (ret->ty != AFS_DIR) {
        errno = ENOTDIR;
        return NULL;
    } else {
        return ret;
    }
}

char *arfs_path(arfile_t *file, size_t *len)
{
    size_t sub;

    if (!file) {
        errno = EINVAL;
        return NULL;
    } else if (!len) {
        len = &sub;
    }

    return arfs_path_r(file, len);
}

inline char *xarfs_path(arfile_t *file, size_t *len)
{
    char *path;
    path = arfs_path(file, len);
    if (!path) {
        errx(1, "invalid file path");
    } else {
        return path;
    }
}

/* mkdir at name. fails if null/invalid args,
 * parent directory does not exist, or file
 * already exists  */
arfile_t *arfs_mkdir(arfile_t *dir, const char *path)
{
    if (!dir || !path || !(dir->fs->mode & AFS_O_WR)) {
        errno = EINVAL;
        return NULL;
    } else {
        return mkfile(dir, path, AE_IFDIR, NULL, 0);
    }
}

arfile_t *arfs_mkfile(arfile_t *dir, const char *path, const uint8_t *buf, size_t len)
{
    if (!dir || !path || !buf) {
        errno = EINVAL;
        return NULL;
    } else {
        return mkfile(dir, path, AE_IFREG, buf, len);
    }
}

arcur_t *arfs_cur(arfile_t *dir, int flags)
{
    return NULL;
}

arfile_t *arfs_next(arcur_t *cur)
{
    return NULL;
}

void arfs_close(arfs_t *fs)
{
    if (fs->aw) {
        ARX(fs->aw, archive_write_close(fs->aw));
        archive_write_free(fs->aw);
    }

    if (fs->ar) {
        ARX(fs->ar, archive_read_close(fs->ar));
        archive_read_free(fs->ar);
    }

    free(fs->path);
    fs->path = NULL;

    free(fs);
}
