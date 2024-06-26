#include <fuse.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>


#define MAX_DATA_IN_FILE 256
#define MAX_PATH_LENGTH 256
#define MAX_NAME_LENGTH 64
#define MAX_INODES 16
#define PERSISENT_FILENAME "filesystem.dat"

#define MAX_DATA_IN_BLOCK 256
#define MAX_BLOCKS 16
#define DIRECT_POINTERS 12

typedef struct DataBlock{
    char data[MAX_DATA_IN_BLOCK];
    bool is_active;
} DataBlock;

//Thread variables
extern pthread_t save_thread;
extern int save_interval;
extern int running;

void* periodic_save();

typedef struct Inode
{
    bool is_active;
    bool is_dir;
    char data[MAX_DATA_IN_FILE];
    char path[MAX_PATH_LENGTH];
    char name[MAX_NAME_LENGTH];
    mode_t mode;
    nlink_t nlink;
    off_t size;
    dev_t devno;
    time_t access_time;
    time_t modif_time;
    uid_t owner;
    gid_t group;
    int direct_pointers[DIRECT_POINTERS];
} Inode;

int dm510fs_getattr( const char *, struct stat * );
int dm510fs_readdir( const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info * );
int dm510fs_open( const char *, struct fuse_file_info * );
int dm510fs_read( const char *, char *, size_t, off_t, struct fuse_file_info * );
int dm510fs_mkdir(const char *path, mode_t mode);
int dm510fs_mknod(const char *path, mode_t mode, dev_t devno);
int dm510fs_utime(const char * path, struct utimbuf *ubuf);
int dm510fs_release(const char *path, struct fuse_file_info *fi);
int dm510fs_rename(const char *path, const char *new_path);
int dm510fs_rmdir(const char *path);
int dm510fs_unlink(const char *path);
int dm510fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info * filp);
int dm510fs_truncate(const char *path, off_t size);
void* dm510fs_init();
void dm510fs_destroy(void *private_data);
