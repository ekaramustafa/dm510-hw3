#include "dm510fs.h"
#include "helper.c"

Inode filesystem[MAX_INODES];
int inode_count; // To track how many inodes are in the filesystem

pthread_t save_thread;
int save_interval = 5; //save the filesystems every 5 seconds
int running = 1;

DataBlock data_blocks[MAX_BLOCKS];

/*
 * See descriptions in fuse source code usually located in /usr/include/fuse/fuse.h
 * Notice: The version on Github is a newer version than installed at IMADA
 */
static struct fuse_operations dm510fs_oper = {
	.getattr = dm510fs_getattr,
	.readdir = dm510fs_readdir,
	.mknod = dm510fs_mknod,
	.mkdir = dm510fs_mkdir,
	.unlink = dm510fs_unlink,
	.rmdir = dm510fs_rmdir,
	.truncate = dm510fs_truncate,
	.open = dm510fs_open,
	.read = dm510fs_read,
	.release = dm510fs_release,
	.write = dm510fs_write,
	.rename = dm510fs_rename,
	.utime = dm510fs_utime,
	.init = dm510fs_init,
	.destroy = dm510fs_destroy
};

/*
 * Return file attributes.
 * The "stat" structure is described in detail in the stat(2) manual page.
 * For the given pathname, this should fill in the elements of the "stat" structure.
 * If a field is meaningless or semi-meaningless (e.g., st_ino) then it should be set to 0 or given a "reasonable" value.
 * This call is pretty much required for a usable filesystem.
*/
int dm510fs_getattr( const char *path, struct stat *stbuf ) {
	printf("getattr: (path=%s)\n", path);

	memset(stbuf, 0, sizeof(struct stat));
	int index = find_active_path_index(filesystem, MAX_INODES, path);
	if(index < 0) return -ENOENT;

	printf("Found inode for path %s, name %s at location %i \n", path, filesystem[index].name, index);
	stbuf->st_mode = filesystem[index].mode;
	stbuf->st_nlink = filesystem[index].nlink;
	stbuf->st_size = filesystem[index].size;
	stbuf->st_dev = filesystem[index].devno;
	stbuf->st_uid = filesystem[index].owner;
	stbuf->st_gid = filesystem[index].group;
	stbuf->st_atime = filesystem[index].access_time;
	stbuf->st_mtime = filesystem[index].modif_time;

	return 0;
}

/*
 * Return one or more directory entries (struct dirent) to the caller. This is one of the most complex FUSE functions.
 * Required for essentially any filesystem, since it's what makes ls and a whole bunch of other things work.
 * The readdir function is somewhat like read, in that it starts at a given offset and returns results in a caller-supplied buffer.
 * However, the offset not a byte offset, and the results are a series of struct dirents rather than being uninterpreted bytes.
 * To make life easier, FUSE provides a "filler" function that will help you put things into the buffer.
 *
 * The general plan for a complete and correct readdir is:
 *
 * 1. Find the first directory entry following the given offset (see below).
 * 2. Optionally, create a struct stat that describes the file as for getattr (but FUSE only looks at st_ino and the file-type bits of st_mode).
 * 3. Call the filler function with arguments of buf, the null-terminated filename, the address of your struct stat
 *    (or NULL if you have none), and the offset of the next directory entry.
 * 4. If filler returns nonzero, or if there are no more files, return 0.
 * 5. Find the next file in the directory.
 * 6. Go back to step 2.
 * From FUSE's point of view, the offset is an uninterpreted off_t (i.e., an unsigned integer).
 * You provide an offset when you call filler, and it's possible that such an offset might come back to you as an argument later.
 * Typically, it's simply the byte offset (within your directory layout) of the directory entry, but it's really up to you.
 *
 * It's also important to note that readdir can return errors in a number of instances;
 * in particular it can return -EBADF if the file handle is invalid, or -ENOENT if you use the path argument and the path doesn't exist.
*/
int dm510fs_readdir( const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ) {
	printf("readdir: (path=%s)\n", path);

	// Check if the directory path exists
	if(find_active_path_index(filesystem, MAX_INODES, path) < 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	for(int i = 0; i < MAX_INODES; i++){
		// Check that it is active and not the exact same path
		if(filesystem[i].is_active && strcmp(filesystem[i].path, path) != 0){
			// Check if the inode is in the same directory path
			char *real_path = extract_path_from_abs(filesystem[i].path);
			printf("real_path: %s\n", real_path);
			if(strcmp(path, real_path) == 0)
				filler(buf, filesystem[i].name, NULL, 0);
		}
	}

	return 0;
}

/*
 * Open a file.
 * If you aren't using file handles, this function should just check for existence and permissions and return either success or an error code.
 * If you use file handles, you should also allocate any necessary structures and set fi->fh.
 * In addition, fi has some other fields that an advanced filesystem might find useful; see the structure definition in fuse_common.h for very brief commentary.
 * Link: https://github.com/libfuse/libfuse/blob/0c12204145d43ad4683136379a130385ef16d166/include/fuse_common.h#L50
*/
int dm510fs_open( const char *path, struct fuse_file_info *fi ) {
    printf("open: (path=%s)\n", path);

	int index = find_active_path_index(filesystem, MAX_INODES, path);
	if(index < 0) return -ENOENT;
	
	return 0;
}

/*
 * Create a new directory
*/
int dm510fs_mkdir(const char *path, mode_t mode) {
	printf("mkdir: (path=%s) (mode=%hu)\n", path, mode);

	int error = handle_inode_creation(filesystem, MAX_INODES, path, inode_count);
	if(error != 0) return error;

	// Locate the first unused Inode in the filesystem
	int free_index = find_inactive_index(filesystem, MAX_INODES, path);
	if(free_index < 0) return -ENOSPC;

	Inode *inode = &filesystem[free_index];
	inode->is_active = true;
	inode->is_dir = true;
	inode->mode = S_IFDIR | mode;
	inode->nlink = 2;
	inode->size = 4096;
	inode->group = getgid();
	inode->owner = getuid();
	inode->access_time = time(NULL);
	inode->modif_time = time(NULL);

	char *name = extract_name_from_abs(path);
	memcpy(inode->name, name, strlen(name) + 1);
	memcpy(inode->path, path, strlen(path) + 1); 			
	inode_count++;

	return 0;
}

int dm510fs_mknod(const char *path, mode_t mode, dev_t devno){
	printf("mknod: (path=%s) (mode=%hu)\n", path, mode);

	int error = handle_inode_creation(filesystem, MAX_INODES, path, inode_count);
	if(error != 0) return error;

	// Locate the first unused Inode in the filesystem
	int free_index = find_inactive_index(filesystem, MAX_INODES, path);
	if(free_index < 0) return -ENOSPC;

	Inode *inode = &filesystem[free_index];
	inode->is_active = true;
	inode->is_dir = false;
	inode->mode = mode;
	inode->nlink = 1;
	inode->devno = devno; // Device number by makedev
	inode->size = 0;
	inode->group = getgid();
	inode->owner = getuid();
	inode->access_time = time(NULL);
	inode->modif_time = time(NULL);
	for(int i =0;i<DIRECT_POINTERS;i++){
		inode->direct_pointers[i] = -1;
	}

	char *name = extract_name_from_abs(path);
	memcpy(inode->name, name, strlen(name) + 1);
	memcpy(inode->path, path, strlen(path) + 1);
	inode_count++;

	return 0;
}

int dm510fs_utime(const char * path, struct utimbuf *ubuf){
	printf("utime: (path=%s)\n",path);
	
	int index = find_active_path_index(filesystem, MAX_INODES, path);
	if(index < 0) return -ENOENT;

	printf("utime: path:%s at location %i\n", path, index);
	filesystem[index].access_time = ubuf->actime;
	filesystem[index].modif_time = ubuf->modtime;
	return 0;
}

int dm510fs_rename(const char *path, const char *new_path) {
    printf("rename : (path=%s)\n", path);

    int count = 0;
    for (int i = 0; i < MAX_INODES; i++) {
        if (filesystem[i].is_active && strncmp(filesystem[i].path, path, strlen(path)) == 0) {

			bool is_subfolder = strlen(filesystem[i].path) > strlen(path);
			char *new_name = extract_name_from_abs(new_path);
			char *prev_name = extract_name_from_abs(path);
			if (!is_subfolder) {
				strcpy(filesystem[i].name, new_name);
				strcpy(filesystem[i].path, new_path);
			} else {
				char result[strlen(filesystem[i].path) - strlen(prev_name) + strlen(new_name) + 1]; // Include space for '\0'
				int insertIndex = 0;
				const char delimiters[] = "/";
				strcpy(result, "/"); // Start with a leading '/'
				insertIndex++;
				char *token = strtok(filesystem[i].path, delimiters); // Initialize strtok with filesystem[i].path
				while (token != NULL) {
					if (strcmp(token, prev_name) == 0) { // Compare token with prev_name
						strcat(result, new_name); // Add new_name to result
						insertIndex += strlen(new_name); 
					} else {
						strcat(result, token); // Add token to result
						insertIndex += strlen(token); // Update insertIndex
					}
					token = strtok(NULL, delimiters);
					if (token != NULL) {
						strcat(result, "/"); // Add '/' if token is not NULL
						insertIndex++;
					}
				}
				strcpy(filesystem[i].path, result); // Update filesystem[i].path with the new path
				free(token);
			}
			count++;
			free(new_name);
			free(prev_name);

        }
    }
    if (count > 0) {
        return 0;
    }
    return -ENOENT;
}

int dm510fs_unlink(const char *path){
	printf("unlink : (path=%s)\n",path);

	int index = find_active_path_index(filesystem, MAX_INODES, path);
	if(index < 0) return -ENOENT;

	filesystem[index].is_active = false;
	for(int j =0; j < DIRECT_POINTERS; j++){
		if(filesystem[index].direct_pointers[j] != -1) {
			deallocate_block(data_blocks, j);
			filesystem[index].direct_pointers[j] = -1;
		}
	}
	inode_count--;
	return 0;
}

int dm510fs_rmdir(const char *path) {
    printf("rmdir: (path=%s)\n", path);
	int count = 0;

    for (int i = 0; i < MAX_INODES; i++) {
		// Check for inodes inside of the directory using strncmp
		bool path_in_directory = strncmp(filesystem[i].path, path, strlen(path)) == 0;
        if (filesystem[i].is_active && path_in_directory) {
            filesystem[i].is_active = false;
			count++;
			inode_count--;
			
        }
    }

	if(count != 0) return 0;

    return -ENOENT; 
}

int dm510fs_truncate(const char *path, off_t size){
    printf("truncate: (path=%s, size=%lld)\n", path, (long long)size);

	int index = find_active_path_index(filesystem, MAX_INODES, path);
	if(index >= 0) {
		filesystem[index].modif_time = time(NULL);
		filesystem[index].size = size;
		return 0;
	}

	return -ENOENT;
}

/* 
 * Write to a file in the filesystem if it is active and has space for the buffer and offset given
*/
/*
int dm510fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info * filp){
	printf("write: (path=%s), (size=%lu), (offset=%ld) \n", path, size, offset);
	
	// Check if the offset plus the size of the buffer does not surpass the max limit
	if(offset + size >= MAX_DATA_IN_FILE){
		printf("The size of data exceeded the limit: %ld > %d \n", size + offset, MAX_DATA_IN_FILE); 
		return -EMSGSIZE;
	}

	int index = find_active_path_index(filesystem, MAX_INODES, path);
	if(index < 0) return -ENOENT;

	if(filesystem[index].size < offset + size) // Adjust the size if it increases
		filesystem[index].size = offset + size;

	memcpy(filesystem[index].data + offset, buf, size);
	filesystem[index].modif_time = time(NULL); // Update modification time
	return size;
}
*/
int dm510fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info * filp) {
    printf("write: (path=%s), (size=%lu), (offset=%ld) \n", path, size, offset);

    int index = find_active_path_index(filesystem, MAX_INODES, path);
    if (index < 0) return -ENOENT;

    Inode *inode = &filesystem[index];

    size_t total_written = 0;
    size_t to_write = size;
    size_t block_offset = offset % MAX_DATA_IN_BLOCK;

    while (to_write > 0) {
        int block_index = offset / MAX_DATA_IN_BLOCK;
        if (block_index < DIRECT_POINTERS) {
            if (inode->direct_pointers[block_index] == -1) {
                inode->direct_pointers[block_index] = allocate_block(data_blocks);
                if (inode->direct_pointers[block_index] == -1) return -ENOSPC;
            }

            int block = inode->direct_pointers[block_index];
            size_t write_size = (to_write < MAX_DATA_IN_BLOCK - block_offset) ? to_write : MAX_DATA_IN_BLOCK - block_offset;
            memcpy(data_blocks[block].data + block_offset, buf + total_written, write_size);

            to_write -= write_size;
            total_written += write_size;
            offset += write_size;
            block_offset = 0;
        } else {
            return -EMSGSIZE;
        }
    }

    inode->size = (inode->size < offset) ? offset : inode->size;
    inode->modif_time = time(NULL);

    return total_written;
}


/*
 * Read size bytes from the given file into the buffer buf, beginning offset bytes into the file. See read(2) for full details.
 * Returns the number of bytes transferred, or 0 if offset was at or beyond the end of the file. Required for any sensible filesystem.
*/
/*
int dm510fs_read( const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi ) {
	printf("read: (path=%s), (size=%lu), (offset=%ld) \n", path, size, offset);

	int index = find_active_path_index(filesystem, MAX_INODES, path);
	if(index < 0) return -ENOENT;

	if(offset >= filesystem[index].size) return 0;

	printf("Read path: %s at location %i\n", path, index);
	// Check if buffer has enough size to receive the data from the offset
	off_t file_size = filesystem[index].size - offset;	
	if(size < file_size) {
		printf("Buffer does not have enough size for the data of the file\n");
		return -ENOBUFS;
	} 

	memcpy(buf, filesystem[index].data + offset, file_size);
	filesystem[index].access_time = time(NULL); // Update access time
	return filesystem[index].size;
}
*/
int dm510fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("read: (path=%s), (size=%lu), (offset=%ld) \n", path, size, offset);

    int index = find_active_path_index(filesystem, MAX_INODES, path);
    if (index < 0) return -ENOENT;

    Inode *inode = &filesystem[index];

    if (offset >= inode->size) return 0;

    size_t total_read = 0;
    size_t to_read = size;
    size_t block_offset = offset % MAX_DATA_IN_BLOCK;

    while (to_read > 0 && offset < inode->size) {
        int block_index = offset / MAX_DATA_IN_BLOCK;
        if (block_index < DIRECT_POINTERS && inode->direct_pointers[block_index] != -1) {
            int block = inode->direct_pointers[block_index];
            size_t read_size = (to_read < MAX_DATA_IN_BLOCK - block_offset) ? to_read : MAX_DATA_IN_BLOCK - block_offset;
            read_size = (read_size < inode->size - offset) ? read_size : inode->size - offset;
            memcpy(buf + total_read, data_blocks[block].data + block_offset, read_size);

            to_read -= read_size;
            total_read += read_size;
            offset += read_size;
            block_offset = 0;
        } else {
            break;
        }
    }

    inode->access_time = time(NULL);

    return total_read;
}


/*
 * This is the only FUSE function that doesn't have a directly corresponding system call, although close(2) is related.
 * Release is called when FUSE is completely done with a file; at that point, you can free up any temporarily allocated data structures.
 */
int dm510fs_release(const char *path, struct fuse_file_info *fi) {
	printf("release: (path=%s)\n", path);
	return 0;
}

/**
 * Initialize filesystem
 *
 * The return value will passed in the `private_data` field of
 * `struct fuse_context` to all file operations, and as a
 * parameter to the destroy() method. It overrides the initial
 * value provided to fuse_main() / fuse_new().
 */
void* dm510fs_init() {
    printf("init filesystem\n");
	inode_count = restore_filesystem(PERSISENT_FILENAME, filesystem, MAX_INODES);
	
	for (int i = 0; i < MAX_BLOCKS; i++) {
        data_blocks[i].is_active = false;
    }
	// Start the periodic save thread
    if (pthread_create(&save_thread, NULL, periodic_save, NULL) != 0) {
        perror("Failed to create save thread");
        exit(EXIT_FAILURE);
    }
    return NULL;
}

/**
 * Clean up filesystem
 * Called on filesystem exit.
 */
void dm510fs_destroy(void *private_data) {
	printf("filesystem unmounted\n");
	running = 0;
    pthread_join(save_thread, NULL);
	save_filesystem(PERSISENT_FILENAME, filesystem, MAX_INODES);
}

void* periodic_save() {
    while (running) {
        sleep(save_interval);
        save_filesystem(PERSISENT_FILENAME, filesystem, MAX_INODES);
		printf("the filesystem periodic save completed.\n");
    }
    return NULL;
}


int main( int argc, char *argv[] ) {
	fuse_main( argc, argv, &dm510fs_oper );

	return 0;
}
