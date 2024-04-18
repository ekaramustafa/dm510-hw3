

char *extract_name_from_abs(const char *buf){
    const char *last_slash = strrchr(buf, '/'); 
    if (last_slash == NULL)
        return NULL;

    return strdup(last_slash + 1);
}


char *extract_path_from_abs(const char *buf) {

    const char *last_slash = strrchr(buf, '/');

    if (last_slash == NULL) {
        return NULL;
    } else if (*(last_slash + 1) == '\0') {
        return strdup("/");
    }

    size_t dir_len = last_slash - buf + 1;

    if(dir_len != 1){
        dir_len--;
    }

    char *dir_path = (char *)malloc(dir_len + 1);
    if (dir_path == NULL) {
        return NULL;
    }

    strncpy(dir_path, buf, dir_len);
    dir_path[dir_len + 1] = '\0'; // Null-terminate the string

    return dir_path;
}




