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
int find_active_path_index(const Inode files[], const int max_files_count, const char *path) {
	for(int i = 0; i < max_files_count; i++){
		if(files[i].is_active && strcmp(files[i].path, path) == 0){
            return i;
        }
    }

    return -1;
}

// Handle error cases for the creation of an inode
int handle_inode_creation(const Inode files[], const int max_files_count, const char *path, const int inode_count) {
    // Check if the path already exists
	if(find_active_path_index(files, max_files_count, path) >= 0) 
		return -EEXIST;
	
	// Check if the number of inodes is not exceeded
	if(inode_count >= max_files_count){
		printf("Cannot create inode\n");
		printf("The limit for number of files reached : %d == %d\n", inode_count, max_files_count);
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
int find_inactive_index(const Inode files[], const int max_files_count, const char *path) {
	for(int i = 0; i < max_files_count; i++){
		if(!files[i].is_active){
            return i;
        }
    }

    return -1;
}

// Create the root inode for the filesystem
void create_root_inode(Inode files[]){
	files[0].is_active = true;
	files[0].is_dir = true;
	files[0].mode = S_IFDIR | 0755;
	files[0].nlink = 2;
    files[0].access_time = time(NULL);
    files[0].modif_time = time(NULL);
    files[0].size = 4096;
	memcpy(files[0].path, "/", 2);
}

// Restore the filesystem from a .dat file created from outside
// Returns the number of inodes in the filesystem, -1 if error occurred
int restore_filesystem(const char *filename, Inode files[], const int max_files_count) {
    // Clear the existing filesystem
    memset(files, 0, max_files_count * sizeof(Inode));

    //Check if the file doesn't exist
    if (access(filename, F_OK) != 0) {
        create_root_inode(files);
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
        if(next_file_index < max_files_count) {
            files[next_file_index] = temp;
            inode_count++;
        } else {
            printf("Max number of inodes reached.\n");
            break;
        }
    }

    fclose(file);
    return inode_count;
}

void save_filesystem(const char *filename, Inode files[], const int max_files_count) {
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
    for (int i = 0; i < max_files_count; i++) {
        if (files[i].is_active) {
            fwrite(&files[i], sizeof(Inode), 1, file);
        }
    }

    fclose(file);
}
