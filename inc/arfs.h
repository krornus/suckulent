#ifndef arfs_h_INCLUDED
#define arfs_h_INCLUDED

#define FS_RD     1
#define FS_WR     2
#define FS_RDWR   3

/* create if not found */
#define FS_CREATE 4

typedef struct arfs arfs_t;
typedef struct ardir ardir_t;
typedef struct arfile arfile_t;

arfs_t *openarfs(const char *path, int flags);

#endif // arfs_h_INCLUDED
