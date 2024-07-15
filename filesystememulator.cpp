#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_FILES 100
#define MAX_FILENAME 100
#define MAX_USERS 10
#define MAX_DIRECTORIES 10
#define DISK_SIZE 1024 * 1024 // 1MB Disk Size

typedef struct {
    char username[50];
} User;

typedef struct {
    char filename[MAX_FILENAME];
    int start_block;
    int size;
    time_t created_time;
    char owner[50];
    char allowed_users[MAX_USERS][50];
    int num_allowed_users;
} File;

typedef struct {
    char name[50];
    File* files[MAX_FILES];
    int file_count;
} Directory;

typedef struct {
    int block_size;
    int total_blocks;
    int free_blocks;
    char* data;
    Directory directories[MAX_DIRECTORIES];
    int dir_count;
    User users[MAX_USERS];
    int user_count;
} FileSystem;

// Initialize the filesystem
FileSystem* init_filesystem(int block_size, int total_blocks) {
    FileSystem* fs = (FileSystem*)malloc(sizeof(FileSystem));
    if (fs == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    fs->block_size = block_size;
    fs->total_blocks = total_blocks;
    fs->free_blocks = total_blocks;
    fs->data = (char*)malloc(block_size * total_blocks);
    if (fs->data == NULL) {
        printf("Memory allocation failed\n");
        free(fs);
        return NULL;
    }
    memset(fs->data, 0, block_size * total_blocks);
    fs->dir_count = 0;
    fs->user_count = 0;
    return fs;
}

//funct to display filesystem info
void display_filesystem_structure(FileSystem* fs) {
    printf("\nCurrent Filesystem Structure:\n");
    for (int i = 0; i < fs->dir_count; i++) {
        printf("Directory: %s\n", fs->directories[i].name);
        if (fs->directories[i].file_count == 0) {
            printf("  (empty)\n");
        }
        else {
            for (int j = 0; j < fs->directories[i].file_count; j++) {
                printf("  - %s\n", fs->directories[i].files[j]->filename);
            }
        }
    }
}

// Create a new user
void create_user(FileSystem* fs, const char* username) {
    if (fs->user_count < MAX_USERS) {
        strcpy(fs->users[fs->user_count].username, username);
        fs->user_count++;
        printf("User %s created successfully\n", username);
    }
    else {
        printf("User limit reached\n");
    }
}


// Create a new directory
void create_directory(FileSystem* fs, const char* dirname) {
    if (fs->dir_count < MAX_DIRECTORIES) {
        strcpy(fs->directories[fs->dir_count].name, dirname);
        fs->directories[fs->dir_count].file_count = 0;
        fs->dir_count++;
        printf("Directory %s created successfully\n", dirname);
    }
    else {
        printf("Directory limit reached\n");
    }
}

// Create a new file in the filesystem
int create_file(FileSystem* fs, const char* dirname, const char* filename, int size, const char* owner, char allowed_users[][50], int num_allowed_users) {
    Directory* dir = NULL;
    for (int i = 0; i < fs->dir_count; i++) {
        if (strcmp(fs->directories[i].name, dirname) == 0) {
            dir = &fs->directories[i];
            break;
        }
    }
    if (dir == NULL) {
        printf("Directory not found\n");
        return -1;
    }
    for (int i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->filename, filename) == 0) {
            printf("File already exists\n");
            return -1;
        }
    }
    if (size > fs->free_blocks * fs->block_size) {
        printf("Not enough space\n");
        return -1;
    }
    File* file = (File*)malloc(sizeof(File));
    strcpy(file->filename, filename);
    file->start_block = fs->total_blocks - fs->free_blocks;
    file->size = size;
    file->created_time = time(NULL);
    strcpy(file->owner, owner);
    file->num_allowed_users = num_allowed_users;
    for (int i = 0; i < num_allowed_users; i++) {
        strcpy(file->allowed_users[i], allowed_users[i]);
    }
    dir->files[dir->file_count] = file;
    dir->file_count++;
    fs->free_blocks -= (size + fs->block_size - 1) / fs->block_size;
    printf("File %s created successfully\n", filename);
    return 0;
}

//write to a file
void write_file(FileSystem* fs, const char* dirname, const char* filename, const char* username, const char* content) {
    Directory* dir = NULL;
    for (int i = 0; i < fs->dir_count; i++) {
        if (strcmp(fs->directories[i].name, dirname) == 0) {
            dir = &fs->directories[i];
            break;
        }
    }
    if (dir == NULL) {
        printf("Directory %s not found.\n", dirname);
        return;
    }

    for (int i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->filename, filename) == 0) {
            if (strcmp(dir->files[i]->owner, username) == 0 ||
                strcmp(username, "admin") == 0) {
                int content_size = strlen(content);
                if (content_size > dir->files[i]->size) {
                    printf("Content is too large for the file.\n");
                    return;
                }
                memcpy(&fs->data[dir->files[i]->start_block * fs->block_size], content, content_size);
                printf("File %s updated successfully.\n", filename);
                return;
            }
            else {
                printf("Access denied. Only the owner or admin can write to this file.\n");
                return;
            }
        }
    }
    printf("File %s not found in directory %s.\n", filename, dirname);
}
// Read a file from the filesystem
void read_file(FileSystem* fs, const char* dirname, const char* filename, const char* username) {
    Directory* dir = NULL;
    for (int i = 0; i < fs->dir_count; i++) {
        if (strcmp(fs->directories[i].name, dirname) == 0) {
            dir = &fs->directories[i];
            break;
        }
    }
    if (dir == NULL) {
        printf("Directory %s not found.\n", dirname);
        return;
    }

    for (int i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->filename, filename) == 0) {
            if (strcmp(dir->files[i]->owner, username) == 0 ||
                strcmp(username, "admin") == 0) {
                printf("Reading file %s:\n", filename);
                for (int j = 0; j < dir->files[i]->size; j++) {
                    printf("%c", fs->data[dir->files[i]->start_block * fs->block_size + j]);
                }
                printf("\n");
                return;
            }
            else {
                for (int j = 0; j < dir->files[i]->num_allowed_users; j++) {
                    if (strcmp(dir->files[i]->allowed_users[j], username) == 0) {
                        printf("Reading file %s:\n", filename);
                        for (int k = 0; k < dir->files[i]->size; k++) {
                            printf("%c", fs->data[dir->files[i]->start_block * fs->block_size + k]);
                        }
                        printf("\n");
                        return;
                    }
                }
                printf("Access denied. User %s is not allowed to read this file.\n", username);
                return;
            }
        }
    }
    printf("File %s not found in directory %s.\n", filename, dirname);
}

// Delete a file from the filesystem
void delete_file(FileSystem* fs, const char* dirname, const char* filename, const char* username) {
    Directory* dir = NULL;
    for (int i = 0; i < fs->dir_count; i++) {
        if (strcmp(fs->directories[i].name, dirname) == 0) {
            dir = &fs->directories[i];
            break;
        }
    }
    if (dir == NULL) {
        printf("Directory %s not found.\n", dirname);
        return;
    }

    for (int i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->filename, filename) == 0) {
            if (strcmp(dir->files[i]->owner, username) == 0 ||
                strcmp(username, "admin") == 0) {
                for (int j = 0; j < dir->files[i]->size; j++) {
                    fs->data[dir->files[i]->start_block * fs->block_size + j] = 0; // Mark blocks as free
                }
                fs->free_blocks += dir->files[i]->size / fs->block_size;
                free(dir->files[i]);
                for (int j = i; j < dir->file_count - 1; j++) {
                    dir->files[j] = dir->files[j + 1];
                }
                dir->file_count--;
                printf("File %s deleted successfully from directory %s.\n", filename, dirname);
                return;
            }
            else {
                printf("Access denied. Only the owner or admin can delete this file.\n");
                return;
            }
        }
    }
    printf("File %s not found in directory %s.\n", filename, dirname);
}

// Print file details
void print_file_details(File* file) {
    printf("Filename: %s\n", file->filename);
    printf("Size: %d bytes\n", file->size);
    printf("Created: %s", ctime(&file->created_time));
    printf("Owner: %s\n", file->owner);
    printf("Allowed users: ");
    for (int i = 0; i < file->num_allowed_users; i++) {
        printf("%s ", file->allowed_users[i]);
    }
    printf("\n");
}

// Print filesystem info
void print_filesystem_info(FileSystem* fs) {
    printf("Filesystem Information:\n");
    printf("Block size: %d bytes\n", fs->block_size);
    printf("Total blocks: %d\n", fs->total_blocks);
    printf("Free blocks: %d\n", fs->free_blocks);
    printf("Total space: %d bytes\n", fs->total_blocks * fs->block_size);
    printf("Free space: %d bytes\n", fs->free_blocks * fs->block_size);
    printf("Number of directories: %d\n", fs->dir_count);
    printf("Number of users: %d\n", fs->user_count);
}

// Print available directories
void print_directories(FileSystem* fs) {
    printf("Available directories:\n");
    for (int i = 0; i < fs->dir_count; i++) {
        printf("%s\n", fs->directories[i].name);
    }
}
//file serialization
void save_filesystem(FileSystem* fs, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }

    fwrite(fs, sizeof(FileSystem), 1, file);
    fwrite(fs->data, fs->block_size * fs->total_blocks, 1, file);

    for (int i = 0; i < fs->dir_count; i++) {
        fwrite(&fs->directories[i], sizeof(Directory), 1, file);
        for (int j = 0; j < fs->directories[i].file_count; j++) {
            fwrite(fs->directories[i].files[j], sizeof(File), 1, file);
        }
    }

    fclose(file);
    printf("File system state saved successfully.\n");
}
//filesystem deserialization
FileSystem* load_filesystem(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("No existing file system state found. Creating a new one.\n");
        return NULL;
    }

    FileSystem* fs = (FileSystem*)malloc(sizeof(FileSystem));
    fread(fs, sizeof(FileSystem), 1, file);

    fs->data = (char*)malloc(fs->block_size * fs->total_blocks);
    fread(fs->data, fs->block_size * fs->total_blocks, 1, file);

    for (int i = 0; i < fs->dir_count; i++) {
        fread(&fs->directories[i], sizeof(Directory), 1, file);
        for (int j = 0; j < fs->directories[i].file_count; j++) {
            fs->directories[i].files[j] = (File*)malloc(sizeof(File));
            fread(fs->directories[i].files[j], sizeof(File), 1, file);
        }
    }

    fclose(file);
    printf("File system state loaded successfully.\n");
    return fs;
}

int main() {
    const char* fs_state_file = "filesystem_state.bin";
    FileSystem* fs = load_filesystem(fs_state_file);

    if (fs == NULL) {
        int block_size = 512;
        int total_blocks = (int)(DISK_SIZE / block_size);
        fs = init_filesystem(block_size, total_blocks);

        create_user(fs, "admin");
        create_user(fs, "user1");
        create_user(fs, "user2");

        create_directory(fs, "root");
        create_directory(fs, "documents");
    }

    char choice;
    char dirname[50];
    char filename[MAX_FILENAME];
    char username[50];
    int size;
    char allowed_users[MAX_USERS][50];
    int num_allowed_users;

    while (1) {
        display_filesystem_structure(fs);  // Add this line to show the structure

        printf("\nFile System Menu:\n");
        printf("1. Create file\n");
        printf("2. Read file\n");
        printf("3. Write to file\n");
        printf("4. Delete file\n");
        printf("5. Print file details\n");
        printf("6. Print filesystem info\n");
        printf("7. Print available directories\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        scanf(" %c", &choice);


        switch (choice) {
        case '1':
            printf("Enter directory name: ");
            scanf("%s", dirname);
            printf("Enter filename: ");
            scanf("%s", filename);
            printf("Enter file size: ");
            scanf("%d", &size);
            printf("Enter owner username: ");
            scanf("%s", username);
            printf("Enter number of allowed users: ");
            scanf("%d", &num_allowed_users);
            for (int i = 0; i < num_allowed_users; i++) {
                printf("Enter allowed user %d: ", i + 1);
                scanf("%s", allowed_users[i]);
            }
            create_file(fs, dirname, filename, size, username, allowed_users, num_allowed_users);
            break;
        case '2':
            printf("Enter directory name: ");
            scanf("%s", dirname);
            printf("Enter filename: ");
            scanf("%s", filename);
            printf("Enter username: ");
            scanf("%s", username);
            read_file(fs, dirname, filename, username);
            break;
        case '3':
            // Write to file
            printf("Enter directory name: ");
            scanf("%s", dirname);
            printf("Enter filename: ");
            scanf("%s", filename);
            printf("Enter username: ");
            scanf("%s", username);
            printf("Enter content: ");
            char content[1024];
            scanf(" %[^\n]", content);
            write_file(fs, dirname, filename, username, content);
            break;
        case '4':
            printf("Enter directory name: ");
            scanf("%s", dirname);
            printf("Enter filename: ");
            scanf("%s", filename);
            printf("Enter username: ");
            scanf("%s", username);
            delete_file(fs, dirname, filename, username);
            break;
        case '5':
            printf("Enter directory name: ");
            scanf("%s", dirname);
            printf("Enter filename: ");
            scanf("%s", filename);
            for (int i = 0; i < fs->dir_count; i++) {
                if (strcmp(fs->directories[i].name, dirname) == 0) {
                    for (int j = 0; j < fs->directories[i].file_count; j++) {
                        if (strcmp(fs->directories[i].files[j]->filename, filename) == 0) {
                            print_file_details(fs->directories[i].files[j]);
                            goto file_found;
                        }
                    }
                }
            }
            printf("File not found.\n");
        file_found:
            break;
        case '6':
            print_filesystem_info(fs);
            break;
        case '7':
            print_directories(fs);
            break;
        case '8':
            // Exit
            printf("Saving file system state...\n");
            save_filesystem(fs, fs_state_file);  // Save state before cleaning up

            // Clean up
            for (int i = 0; i < fs->dir_count; i++) {
                for (int j = 0; j < fs->directories[i].file_count; j++) {
                    free(fs->directories[i].files[j]);
                }
            }
            free(fs->data);
            free(fs);
            return 0;
        default:
            printf("Invalid choice. Please try again.\n");
        }

    }

    return 0;
}
