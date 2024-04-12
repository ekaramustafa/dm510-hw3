#include "dm510fs.h"
#include "helper.c"

Inode filesystem[MAX_INODES];

/*
 * See descriptions in fuse source code usually located in /usr/include/fuse/fuse.h
 * Notice: The version on Github is a newer version than installed at IMADA
 */
static struct fuse_operations dm510fs_oper = {
	.getattr	= dm510fs_getattr,
	.readdir	= dm510fs_readdir,
	.mknod = dm510fs_mknod,
	.mkdir = dm510fs_mkdir,
	.unlink = dm510fs_unlink,
	.rmdir = dm510fs_rmdir,
	.truncate = dm510fs_truncate,
	.open	= dm510fs_open,
	.read	= dm510fs_read,
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

	if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

	for(int i =0;i < MAX_INODES;i++){
		if(filesystem[i].is_active){
			if(strcmp(filesystem[i].path, path) == 0){
				printf("Found inode for path %s, name %s at location %i \n",path,filesystem[i].name,i);
				stbuf->st_mode = filesystem[i].mode;
				stbuf->st_nlink = filesystem[i].nlink;
				stbuf->st_size = filesystem[i].size;
				stbuf->st_dev = filesystem[i].devno;
				stbuf->st_uid = filesystem[i].owner;
				stbuf->st_gid = filesystem[i].group;
				stbuf->st_atime = filesystem[i].atime;
				stbuf->st_mtime = filesystem[i].mtime;
				return 0;	
			}
		}
		
	}
	return -ENOENT;
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
	(void) offset;
	(void) fi;
	printf("readdir: (path=%s)\n", path);
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	for(int i =0;i<MAX_INODES;i++){
		if(filesystem[i].is_active){
			char *real_path = extract_path_from_abs(filesystem[i].path);
			if(strcmp(path, real_path) == 0){
				if(strlen(filesystem[i].name) != 0){
					filler(buf, filesystem[i].name, NULL, 0);
				}				
			}
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
	return 0;
}

int dm510fs_mkdir(const char *path, mode_t mode) {
	printf("mkdir: (path=%s)\n", path);
	// Locate the first unused Inode in the filesystem
	for( int i = 0; i < MAX_INODES; i++) {
		if( filesystem[i].is_active == false ) {
			printf("mkdir: Found unused inode for at location %i\n", i);
			// Use that for the directory
			filesystem[i].is_active = true;
			filesystem[i].is_dir = true;
			filesystem[i].mode = S_IFDIR | 0755;
			filesystem[i].nlink = 2;
			filesystem[i].size = 4096;
			filesystem[i].group = getgid();
			filesystem[i].owner = getuid();
			time_t current_time = time(NULL);
			filesystem[i].atime = current_time;
			filesystem[i].mtime = current_time;
			char *name = extract_name_from_abs(path);
			memcpy(filesystem[i].name, name, strlen(name) + 1);
			memcpy(filesystem[i].path, path, strlen(path) + 1); 			
			return 0;
		}
	}

	return -ENONET;
}

int dm510fs_mknod(const char *path, mode_t mode, dev_t devno){
	printf("mknode: (path=%s)\n",path);
	for( int i = 0; i < MAX_INODES; i++) {
		if( filesystem[i].is_active == false ) {
			printf("mknod: Found unused inode for at location %i\n", i);
			filesystem[i].is_active = true;
			filesystem[i].is_dir = false;
			filesystem[i].mode = S_IFREG | 0777;
			filesystem[i].nlink = 1;
			filesystem[i].devno = devno; //device number by makedev
			filesystem[i].size = 0;
			filesystem[i].group = getgid();
			filesystem[i].owner = getuid();
			time_t current_time = time(NULL);
			filesystem[i].atime = current_time;
			filesystem[i].mtime = current_time;
			char *name = extract_name_from_abs(path);
			memcpy(filesystem[i].name, name, strlen(name) + 1);
			memcpy(filesystem[i].path, path, strlen(path)+1); 			
			return 0;
		}
	}

	return -ENOENT;
}

//TO-DO memory leaks and error handling
int dm510fs_rename(const char *path, const char *new_path) {
    printf("rename : (path=%s)\n", path);
    int count = 0;
    for (int i = 0; i < MAX_INODES; i++) {
        if (filesystem[i].is_active) {
            bool is_subfolder = strlen(filesystem[i].path) > strlen(path);
            if (strncmp(filesystem[i].path, path, strlen(path)) == 0) {
                char *new_name = extract_name_from_abs(new_path);
                char *prev_name = extract_name_from_abs(path);
                if (!is_subfolder) {
                    strcpy(filesystem[i].name, new_name); // Use strcpy to copy the string
					strcpy(filesystem[i].path, new_path); // Use strcpy to copy the string
                } else {
                    char result[strlen(filesystem[i].path) - strlen(prev_name) + strlen(new_name) + 1]; // Include space for '\0'
                    int insertIndex = 0;
                    const char delimiters[] = "/";
                    strcpy(result, "/"); // Start with a leading '/'
                    insertIndex++;
                    char *token = strtok(filesystem[i].path, delimiters); // Initialize strtok with filesystem[i].path
                    while (token != NULL) {
                        if (strcmp(token, prev_name) == 0) { // Compare token with prev_name
                            strcat(result, new_name); // ADd new_name to result
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
    }
    if (count > 0) {
        return 0;
    }
    return -ENOENT;
}

int dm510fs_utime(const char * path, struct utimbuf *ubuf){
	printf("utime: (path=%s)\n",path);
	for(int i =0;i < MAX_INODES;i++){
		if(filesystem[i].is_active){
			if(strcmp(filesystem[i].path, path) == 0){
				printf("utime: path :%s at location %i\n",path,i);
				filesystem[i].atime = ubuf->actime;
				filesystem[i].mtime = ubuf->modtime;
				printf("name : %s\n",filesystem[i].name);
				return 0;
			}
		}
	}

	return -ENOENT;
}

int dm510fs_unlink(const char *path){
	printf("unlink : (path=%s)\n",path);
	for(int i =0;i<MAX_INODES;i++){
		if(filesystem[i].is_active){
			if(strcmp(filesystem[i].path, path) == 0){
				filesystem[i].is_active = false;
				return 0;
			}
		}
	}

	return -ENOENT;
}


int dm510fs_rmdir(const char *path) {
    printf("rmdir: (path=%s)\n", path);
	int count = 0;
    for (int i = 0; i < MAX_INODES; i++) {
		//to check subfolders of directory use strncmp
        if (filesystem[i].is_active && strncmp(filesystem[i].path, path, strlen(path)) == 0) {
            filesystem[i].is_active = false;
			count++;
        }
    }
	if(count != 0){
		return 0;
	}
    return -ENOENT; 
}


int dm510fs_truncate(const char *path, off_t size){
    printf("truncate: (path=%s, size=%lld)\n", path, (long long)size);

	for (int i = 0; i < MAX_INODES; i++) {
        if (filesystem[i].is_active && strcmp(filesystem[i].path, path) == 0) {
            filesystem[i].mtime = time(NULL);
			filesystem[i].size = size;
            return 0;
        }
    }


	return -ENOENT;
}

int dm510fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info * filp){
	printf("write: (path=%s), (size=%lu), (offset=%ld) \n", path, size, offset);
	for(int i =0;i< MAX_INODES;i++){
		if(filesystem[i].is_active){
			if(strcmp(filesystem[i].path, path) == 0){
				if(offset + size > filesystem[i].size) {
                // Adjust file size if necessary
                filesystem[i].size = offset + size;
            	}
            	// Copy data to the file's data buffer at the specified offset
            	memcpy(filesystem[i].data + offset, buf, size);
            	filesystem[i].mtime = time(NULL); // Update modification time
            	return size;
			}
		}
	}

	return -ENOENT;
}

/*
 * Read size bytes from the given file into the buffer buf, beginning offset bytes into the file. See read(2) for full details.
 * Returns the number of bytes transferred, or 0 if offset was at or beyond the end of the file. Required for any sensible filesystem.
*/
int dm510fs_read( const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi ) {
    printf("read: (path=%s)\n", path);

	for(int i =0; i< MAX_INODES;i++){

		if(strcmp(filesystem[i].path, path) == 0){
			printf("Read : path : %s at location %i\n", path, i);
			memcpy(buf, filesystem[i].data, filesystem[i].size);
			filesystem[i].atime = time(NULL);
			return filesystem[i].size;
		}

	}
	return 0;
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

	// Loop through all inodes - set them inactive
	for( int i = 0; i < MAX_INODES; i++) {
		filesystem[i].is_active = false;
	}
	time_t current_time = time(NULL);
	// Add root inode 
	filesystem[0].is_active = true;
	filesystem[0].is_dir = true;
	filesystem[0].mode = S_IFDIR | 0755;
	filesystem[0].nlink = 2;
	filesystem[0].group = getgid();
	filesystem[0].owner = getuid();
	filesystem[0].size = 4096; //from an observation on my laptop
	filesystem[0].atime = current_time;
	filesystem[0].mtime = current_time;
	memcpy(filesystem[0].path, "/", 2); 

	// Add inode for the hello file
	filesystem[1].is_active = true;
	filesystem[1].is_dir = false;
	filesystem[1].mode = S_IFREG | 0777;
	filesystem[1].nlink = 1;
	filesystem[1].size = 12;
	filesystem[1].group = getgid();
	filesystem[1].owner = getuid();
	filesystem[1].atime = current_time;
	filesystem[1].mtime = current_time;
	memcpy(filesystem[1].path, "/hello", 7);
	memcpy(filesystem[1].data, "Hello World!", 13);
	memcpy(filesystem[1].name, "hello", 6);
    return NULL;
}

/**
 * Clean up filesystem
 * Called on filesystem exit.
 */
void dm510fs_destroy(void *private_data) {
    printf("destroy filesystem\n");
}


int main( int argc, char *argv[] ) {
	fuse_main( argc, argv, &dm510fs_oper );

	return 0;
}
