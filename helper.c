char *extract_name_from_abs(const char *path){
    const char *last_slash = strrchr(path, '/'); 
    if (last_slash == NULL)
        return NULL;

    return strdup(last_slash + 1);
}

char *extract_path_from_abs(const char *path) {
    const char *last_slash = strrchr(path, '/');

    if (last_slash == NULL) {
        return NULL;
    } else if (*(last_slash + 1) == '\0') {
        return strdup("/");
    }

    size_t dir_len = last_slash - path + 1;

    if(dir_len != 1){
        dir_len--;
    }

    char *dir_path = (char *)malloc(dir_len + 1);
    if (dir_path == NULL) {
        return NULL;
    }

    strncpy(dir_path, path, dir_len);
    dir_path[dir_len + 1] = '\0'; // Null-terminate the string

    return dir_path;
}

// Returns the index of the found path in the filesystem provided if this is active
// Returns -1 if the file was not found or if it is not active (deleted)
// fs -> filesystem
int find_active_path_index(const Inode fs[], const int fs_max_size, const char *path) {
	for(int i = 0; i < fs_max_size; i++){
		if(fs[i].is_active && strcmp(fs[i].path, path) == 0){
            return i;
        }
    }

    return -1;
}

// Handle error cases for the creation of an inode
int handle_inode_creation(const Inode fs[], const int fs_max_size, const char *path, const int inode_count) {
    // Check if the path already exists
	if(find_active_path_index(fs, fs_max_size, path) >= 0) 
		return -EEXIST;
	
	// Check if the number of inodes is not exceeded
	if(inode_count >= fs_max_size){
		printf("Cannot create inode\n");
		printf("The limit for number of files reached : %d == %d\n", inode_count, fs_max_size);
		return -ENOSPC;
	}

    // Takes into account the null terminating character
	size_t path_length = strlen(path) + 1;
	size_t name_length = strlen(extract_name_from_abs(path)) + 1;

	if(name_length > MAX_NAME_LENGTH){
		printf("Cannot create inode\n");
		printf("The length of filename exceeded the limit: %ld > %d \n", name_length, MAX_NAME_LENGTH); 
		return -ENAMETOOLONG;
	}

	if(path_length > MAX_PATH_LENGTH){
		printf("Cannot create inode\n");
		printf("The length of path exceeded the limit: %ld > %d \n", path_length, MAX_PATH_LENGTH); 
		return -ENAMETOOLONG;
	}

    return 0;
}

// Returns the index of the first inactive node in the filesystem
// Returns -1 if every node is active
// fs -> filesystem
int find_inactive_index(const Inode fs[], const int fs_max_size, const char *path) {
	for(int i = 0; i < fs_max_size; i++){
		if(!fs[i].is_active){
            return i;
        }
    }

    return -1;
}

// Create the root inode for the filesystem
void create_root_inode(Inode fs[]){
	fs[0].is_active = true;
	fs[0].is_dir = true;
	fs[0].mode = S_IFDIR | 0755;
	fs[0].nlink = 2;
    fs[0].access_time = time(NULL);
    fs[0].modif_time = time(NULL);
    fs[0].size = 4096;
	memcpy(fs[0].path, "/", 2);
}

// Restore the filesystem from a .dat file created from outside
// Returns the number of inodes in the filesystem, -1 if error occurred
// fs -> filesystem
int restore_filesystem(const char *filename, Inode fs[], const int fs_max_size) {
    // Clear the existing filesystem
    memset(fs, 0, fs_max_size * sizeof(Inode));

    //Check if the file doesn't exist
    if (access(filename, F_OK) != 0) {
        create_root_inode(fs);
        printf("Creating filesystem file...\n");
        // Create filesystem file
        FILE *file = fopen(filename, "w");
        if(file == NULL) {
            perror("Error creating filesystem file");
            return -1;
        }
        fclose(file);
        return 1;
    }

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file for reading");
        return -1;
    }

    // Read each inode from the file and populate the filesystem array
    Inode temp;
    int next_file_index = -1;
    int inode_count = 0;
    while(fread(&temp, sizeof(Inode), 1, file) == 1) {
        next_file_index++;
        if(next_file_index < fs_max_size) {
            fs[next_file_index] = temp;
            inode_count++;
        } else {
            printf("Max number of inodes reached.\n");
            break;
        }
    }

    fclose(file);
    return inode_count;
}

void save_filesystem(const char *filename, Inode fs[], const int fs_max_size) {
    //Check if the file doesn't exist
    if (access(filename, F_OK) != 0) {
        printf("Filesystem file does not exist.\n");
        return;
    }

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    // Write each active inode to the file
    for (int i = 0; i < fs_max_size; i++) {
        if (fs[i].is_active) {
            fwrite(&fs[i], sizeof(Inode), 1, file);
        }
    }

    fclose(file);
}


int allocate_block(DataBlock data_blocks[]) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!data_blocks[i].is_active) {
            data_blocks[i].is_active = true;
            return i;
        }
    }
    return -1; // No free blocks
}

void deallocate_block(DataBlock data_blocks[], int index) {
    if (index >= 0 && index < MAX_BLOCKS) {
        data_blocks[index].is_active = false;
    }
}
