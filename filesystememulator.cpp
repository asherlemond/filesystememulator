#include "filesystem_emulator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCK_SIZE 4096
#define MAX_BLOCKS 1024
#define MAX_FILES 64
#define MAX_FILENAME 32

// File system structures

typedef struct {
    char name[MAX_FILENAME];
    int size;
    int start_block;
    time_t created;
    time_t modified;
} inode;

typedef struct {
    int total_blocks;
    int free_blocks;
    int total_inodes;
    int free_inodes;
} superblock;

typedef struct {
    superblock sb;
    inode inodes[MAX_FILES];
    char blocks[MAX_BLOCKS][BLOCK_SIZE];
    int block_map[MAX_BLOCKS];
} filesystem;

// Function prototypes
void init_filesystem(filesystem* fs);
int allocate_block(filesystem* fs);
void free_block(filesystem* fs, int block);
int create_file(filesystem* fs, const char* filename);
int write_to_file(filesystem* fs, const char* filename, const char* data);
char* read_from_file(filesystem* fs, const char* filename);
void delete_file(filesystem* fs, const char* filename);
void list_files(filesystem* fs);

// Initialize the filesystem
void init_filesystem(filesystem* fs) {
    fs->sb.total_blocks = MAX_BLOCKS;
    fs->sb.free_blocks = MAX_BLOCKS;
    fs->sb.total_inodes = MAX_FILES;
    fs->sb.free_inodes = MAX_FILES;

    memset(fs->block_map, 0, sizeof(fs->block_map));
    memset(fs->inodes, 0, sizeof(fs->inodes));
}

// Allocate a free block
int allocate_block(filesystem* fs) {
    if (fs->sb.free_blocks == 0) return -1;

    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (fs->block_map[i] == 0) {
            fs->block_map[i] = 1;
            fs->sb.free_blocks--;
            return i;
        }
    }
    return -1;
}

// Free a block
void free_block(filesystem* fs, int block) {
    if (block >= 0 && block < MAX_BLOCKS) {
        fs->block_map[block] = 0;
        fs->sb.free_blocks++;
    }
}
// Create a new file
int create_file(filesystem* fs, const char* filename) {
    if (fs->sb.free_inodes == 0) return -1;

    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->inodes[i].name[0] == '\0') {
            strncpy_s(fs->inodes[i].name, MAX_FILENAME, filename, MAX_FILENAME - 1);
            fs->inodes[i].name[MAX_FILENAME - 1] = '\0'; // Ensure null termination
            fs->inodes[i].size = 0;
            fs->inodes[i].start_block = -1;
            fs->inodes[i].created = time(NULL);
            fs->inodes[i].modified = time(NULL);
            fs->sb.free_inodes--;
            return i;
        }
    }
    return -1;
}

// Write data to a file
int write_to_file(filesystem* fs, const char* filename, const char* data) {
    int inode_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (strcmp(fs->inodes[i].name, filename) == 0) {
            inode_index = i;
            break;
        }
    }

    if (inode_index == -1) return -1;

    size_t data_size = strlen(data);
    int blocks_needed = (int)((data_size + BLOCK_SIZE - 1) / BLOCK_SIZE);

    if (blocks_needed > fs->sb.free_blocks) return -1;

    // ... (rest of the function remains the same)
}

// Read data from a file
char* read_from_file(filesystem* fs, const char* filename) {
    int inode_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (strcmp(fs->inodes[i].name, filename) == 0) {
            inode_index = i;
            break;
        }
    }

    if (inode_index == -1) return NULL;

    char* data = (char*)malloc(fs->inodes[inode_index].size + 1);
    if (!data) return NULL;

    // ... (rest of the function remains the same)
}

// Delete a file
void delete_file(filesystem* fs, const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (strcmp(fs->inodes[i].name, filename) == 0) {
            int current_block = fs->inodes[i].start_block;
            while (current_block != -1) {
                int next_block = *(int*)&fs->blocks[current_block];
                free_block(fs, current_block);
                current_block = next_block;
            }

            memset(&fs->inodes[i], 0, sizeof(inode));
            fs->sb.free_inodes++;
            break;
        }
    }
}

// List all files
void list_files(filesystem* fs) {
    printf("Files in the filesystem:\n");
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->inodes[i].name[0] != '\0') {
            printf("%s (size: %d bytes)\n", fs->inodes[i].name, fs->inodes[i].size);
        }
    }
}

int main() {
    filesystem fs;
    init_filesystem(&fs);

    create_file(&fs, "example.txt");
    write_to_file(&fs, "example.txt", "Hello, this is a test file content.");

    create_file(&fs, "another.txt");
    write_to_file(&fs, "another.txt", "This is another file with different content.");

    list_files(&fs);

    char* content = read_from_file(&fs, "example.txt");
    if (content) {
        printf("Content of example.txt: %s\n", content);
        free(content);
    }

    delete_file(&fs, "another.txt");

    printf("After deleting another.txt:\n");
    list_files(&fs);

    return 0;
}