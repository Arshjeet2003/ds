#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>

#define MAX_FILES 1024
#define PATH_MAX_LEN 4096
#define USER_MAX_LEN 256

// Structure to hold file information
typedef struct {
    char filename[PATH_MAX_LEN];
    char owner[USER_MAX_LEN];
    off_t size;
} FileInfo;

// Map Phase: Collect file sizes, owners, and filenames
int map_phase(const char *directory, FileInfo *files, int *file_count) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    struct passwd *pwd;

    dir = opendir(directory);
    if (!dir) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        char filepath[PATH_MAX_LEN];
        snprintf(filepath, PATH_MAX_LEN, "%s/%s", directory, entry->d_name);

        // Skip directories
        if (stat(filepath, &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
            // Get file information
            pwd = getpwuid(file_stat.st_uid);
            if (pwd == NULL) {
                perror("getpwuid");
                continue;
            }

            // Store file information
            strncpy(files[*file_count].filename, entry->d_name, PATH_MAX_LEN);
            strncpy(files[*file_count].owner, pwd->pw_name, USER_MAX_LEN);
            files[*file_count].size = file_stat.st_size;
            (*file_count)++;

            if (*file_count >= MAX_FILES) {
                fprintf(stderr, "Too many files. Increase MAX_FILES.\n");
                break;
            }
        }
    }

    closedir(dir);
    return 0;
}

// Reduce Phase: Find users who own the files with the maximum size
void reduce_phase(FileInfo *files, int file_count) {
    off_t max_size = 0;

    // Find the maximum file size
    for (int i = 0; i < file_count; i++) {
        if (files[i].size > max_size) {
            max_size = files[i].size;
        }
    }

    // Print users and files with the maximum size
    printf("Files with the maximum size (%lld bytes):\n", max_size);
    for (int i = 0; i < file_count; i++) {
        if (files[i].size == max_size) {
            printf("  Owner: %s, File: %s\n", files[i].owner, files[i].filename);
        }
    }
}

int main() {
    char cwd[PATH_MAX_LEN];
    FileInfo files[MAX_FILES];
    int file_count = 0;

    // Get the current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    printf("Current working directory: %s\n", cwd);

    // Perform the Map Phase
    if (map_phase(cwd, files, &file_count) != 0) {
        fprintf(stderr, "Error during map phase.\n");
        return EXIT_FAILURE;
    }

    if (file_count == 0) {
        printf("No files found in the current directory.\n");
        return EXIT_SUCCESS;
    }

    // Perform the Reduce Phase
    reduce_phase(files, file_count);

    return EXIT_SUCCESS;
}
