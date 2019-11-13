#ifndef arfs_h_INCLUDED
#define arfs_h_INCLUDED

#include <sys/types.h>
#include <stdint.h>

enum ARFSOpenFlags {
    AFS_O_RD = 1,
    AFS_O_WR = 2,
    AFS_O_RDWR = AFS_O_RD | AFS_O_WR,
    AFS_O_CREATE = 4, /* create if the file does not exist */
};

enum ARFSIterFlags {
    AFS_IT_R = 1, /* recurse */
    AFS_IT_F = 2, /* include files */
    AFS_IT_D = 4, /* include directories */
    AFS_IT_L = 8, /* include links */
    AFS_IT_ALL = AFS_IT_R | AFS_IT_F | AFS_IT_D,
};

typedef struct arfs arfs_t;
typedef struct arfile arfile_t;
typedef struct arcur arcur_t;

arfs_t *arfs_open(const char *path, int flags);
void arfs_close(arfs_t *fs);

arfile_t *arfs_root(arfs_t *fs);
char *arfs_path(arfile_t *file, size_t *len);
char *xarfs_path(arfile_t *file, size_t *len);
arfile_t *arfs_dirof(arfile_t *dir, const char *path);

arfile_t *arfs_file(arfile_t *dir, const char *path);
arfile_t *arfs_dir(arfile_t *dir, const char *path);
arfile_t *arfs_mkdir(arfile_t *dir, const char *path);
arfile_t *arfs_mkfile(arfile_t *dir, const char *path, const uint8_t *buf, size_t len);

arcur_t *arfs_cur(arfile_t *dir, int flags);
arfile_t *arfs_next(arcur_t *cur);

#endif // arfs_h_INCLUDED
