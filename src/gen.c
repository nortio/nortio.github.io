#define _POSIX_C_SOURCE 200809L
#include "dyn_array.h"
#include "md4c-html.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

Array(char);

void process(const MD_CHAR *data, MD_SIZE size, void *userdata) {
    Array_char *out = userdata;
    fprintf(stdout, "L: %.*s\n", size, data);
    char_array_push_multiple(out, (char*)data, size);
}

int read_and_parse(int fd, size_t file_size, Array_char *out) {
    const char *file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    if (close(fd)) {
        perror("close");
        return 1;
    }

    if (md_html(file_data, file_size, process, out, 0, 0)) {
        fprintf(stderr, "md_html failed\n");
        return 1;
    }

    if (munmap((void *)file_data, file_size)) {
        perror("munmap");
        return 1;
    }

    return 0;
}

int main(int argc, char **argv) {
    const char *content_dir_path = "./blog/";
    Array_char out = ArrayInitializer;

    DIR *content_dir = opendir(content_dir_path);
    if (!content_dir) {
        perror("opendir");
        return 1;
    }
    int content_dir_fd = dirfd(content_dir);
    if (!content_dir_fd) {
        perror("dirfd");
        return 1;
    }

    struct dirent *ent = NULL;
    while ((ent = readdir(content_dir), ent)) {
        if (ent->d_name[0] == '.') {
            continue;
        }

        struct stat f = {0};
        if (fstatat(content_dir_fd, ent->d_name, &f, 0)) {
            perror("fstat");
            return 1;
        }

        if (S_ISLNK(f.st_mode)) {
            continue;
        }

        bool is_dir = S_ISDIR(f.st_mode);

        printf("dirent: %s, is_dir: %d\n", ent->d_name, is_dir);

        if (is_dir) {
            // int post_dir_fd = openat(content_dir_fd, ent->d_name, O_RDONLY | O_DIRECTORY);
            // if(post_dir_fd == -1) {
            //     perror("openat post dir");
            //     return 1;
            // }

            continue;
        }

        int fd = openat(content_dir_fd, ent->d_name, O_RDONLY);
        if (fd == -1) {
            perror("openat");
            return 1;
        }

        if (f.st_size == 0) {
            continue;
        }

        if (read_and_parse(fd, f.st_size, &out)) {
            fprintf(stderr, "read_and_parse failed!\n");
            return 1;
        }
    }

    fprintf(stdout, "size: %lu\n%.*s", out.count, (int)out.count, out.data);

    if (closedir(content_dir)) {
        perror("closedir");
        return 1;
    }
}
