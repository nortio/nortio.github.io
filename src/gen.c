#define _POSIX_C_SOURCE 200809L
#include <errno.h>
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

typedef struct Span {
    size_t len;
    const char* data;
} Span;

int writeall(FILE* file, Span data) {
    size_t written = 0;
    do {
        written = fwrite(data.data, sizeof((*data.data)), data.len, file);
        if(written == 0) {
            perror("fwrite");
            return 1;
        }
    } while (data.len - written);

    return 0;
}

/**
 * Closes the fd after mmap of the file
 */
Span fdreadall(int fd, size_t size, int* err) {
    const char *file_data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("fdreadall: mmap");
        *err = errno;
        return (Span){
            .data = NULL,
            .len = 0
        };
    }

    if (close(fd)) {
        perror("fdreadall: close");
        *err = errno;
        return (Span){
            .data = NULL,
            .len = 0
        };
    }

    *err = 0;
    return (Span) {
        .data = file_data,
        .len = size
    };
}

Span readall(const char* name, int* err) {
    int fd = open(name, O_RDONLY);
    if(fd == -1) {
        *err = errno;
        return (Span){
            .data = NULL,
            .len = 0
        };
    }

    struct stat s = {0};
    if(fstat(fd, &s)) {
        *err = errno;
        return (Span){
            .data = NULL,
            .len = 0
        };
    }

    *err = 0;
    return fdreadall(fd, s.st_size, err);
}

int closeall(Span *file_data) {
    if (munmap((void *)file_data->data, file_data->len)) {
        file_data->data = NULL;
        file_data->len = 0;
        return 1;
    }

    file_data->data = NULL;
    file_data->len = 0;

    return 0;
}

void process(const MD_CHAR *data, MD_SIZE size, void *userdata) {
    Array_char *out = userdata;
    fprintf(stdout, "L: %.*s\n", size, data);
    char_array_push_multiple(out, (char *)data, size);
}

int read_and_parse(int fd, size_t file_size, Array_char *out) {
    int err = 0;
    Span file_data = fdreadall(fd, file_size, &err);
    if(err) {
        perror("read_and_parse");
        return 1;
    }

    if (md_html(file_data.data, file_data.len, process, out, 0, 0)) {
        fprintf(stderr, "md_html failed\n");
        return 1;
    }

    if (closeall(&file_data)) {
        perror("fdcloseall");
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

    int err = 0;
    Span top = readall("top.html", &err);
    if(err) {
        perror("readall");
        return 1;
    }

    Span bottom = readall("bottom.html", &err);
    if(err) {
        perror("readall");
        return 1;
    }

    fprintf(stdout, "size: %lu\n%.*s", out.count, (int)out.count, out.data);
    FILE *index_html_file = fopen("./out/index.html", "w+");
    if (!index_html_file) {
        perror("fopen");
        return 1;
    }

    writeall(index_html_file, top);
    writeall(index_html_file, (Span){.data = out.data, .len = out.count});
    writeall(index_html_file, bottom);

    if(fclose(index_html_file)) {
        perror("fclose");
        return 1;
    }

    if (closedir(content_dir)) {
        perror("closedir");
        return 1;
    }
}
