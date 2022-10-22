#define _GNU_SOURCE
#define INODE_COUNT 1024
#define FILE_IDX 1
#define NUM_BYTES 100
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>


void *checked_malloc(int len) {
    void *p = malloc(len);
    
    if (p == NULL) {
        perror("\nRan out of memory!\n");
        exit(1);
    }
    return p;
}

char *uint32_to_str(uint32_t i)
{
   int length = snprintf(NULL, 0, "%lu", (unsigned long)i);       // pretend to print to a string to get length
   char* str = checked_malloc(length + 1);                        // allocate space for the actual string
   snprintf(str, length + 1, "%lu", (unsigned long)i);            // print to string
   return str;
}

FILE *open_file(char *name, char*mode) {
    FILE *file;
    file = fopen(name, mode);
    if (file == NULL) {
        perror(name);
        exit(1);
    }
    return file;
}

void list_contents(char *curr_directory) {
    FILE *directory_file = open_file(curr_directory, "r");
    uint32_t inode = 0;
    char name[34] = "";

    while (fread(&inode, sizeof(int), 1, directory_file) == 1) {
        if (fread(&name, 32, 1, directory_file) == 1) {
        }
            printf("%d ", inode);
            printf(" %s \n", name);
    }
    fclose(directory_file);
}

void update_directory(char *inodes, char *curr_directory, char *new_directory) {
    FILE *directory_file = open_file(curr_directory, "r");
    uint32_t inode;
    char name[34] = "";
    int updated = 0;

    while (fread(&inode, sizeof(int), 1, directory_file) == 1) {
        if (fread(&name, 32, 1, directory_file) == 1) {
        }
            strcat(name, "\n");

            char *temp_name = uint32_to_str(inode);
            if (strcmp(new_directory, name) == 0 && inodes[inode] == 'd') {
                strcpy(curr_directory, temp_name);
                updated = 1;
            }
            free(temp_name);
            
    }
    if (updated == 0) {
        printf("Not a directory\n");
    }
    fclose(directory_file);
}

void make_directory(char *inodes, size_t *inodes_length, char *curr_directory, char *new_directory) {
    //check if there's an available inode
    uint32_t curr_inode = strtoul(curr_directory, NULL, 10);
    uint32_t *curr_inode_ptr = &curr_inode; 
    char name[34] = "";
    uint32_t inode = 0;

    char truncated_name[32] = "";

    int i = 0;
    if (*inodes_length < 1023) {
        ++(*inodes_length);
        uint32_t new_inode = *inodes_length - 1;
        uint32_t *new_inode_ptr = &new_inode;
        inodes[new_inode] = 'd';
        //open the curr dir and write to it 
        //trunc name length to 32 chars

        if (strlen(new_directory) > 32) {
            for (i = 0; i < 32; i++) {
                truncated_name[i] = new_directory[i];
            }
        }
        else {
            for (i = 0; i < strlen(new_directory) - 1; i++) {
                truncated_name[i] = new_directory[i];
            }
                truncated_name[i] = '\0';
        }

        //check if directory name already exists in curr directory
        FILE *directory_file = open_file(curr_directory, "a+");

        while (fread(&inode, sizeof(int), 1, directory_file) == 1) {
            if (fread(&name, 32, 1, directory_file) == 1) {
            }
            strcat(name, "\n");
            if (strcmp(new_directory, name) == 0 && inodes[inode] == 'd') {
                printf("directory with that name already exists\n");
                return;
            }
        }
        
        fwrite(new_inode_ptr, 4, 1, directory_file);
        fwrite(truncated_name, 32, 1, directory_file);
        fclose(directory_file);

        char *temp_name = uint32_to_str(new_inode);
        FILE *new_dir_file = open_file(temp_name, "w");
        fwrite(new_inode_ptr, 4, 1, new_dir_file);
        fwrite(".", 32, 1, new_dir_file);
        fwrite(curr_inode_ptr, 4, 1, new_dir_file);
        fwrite("..", 32, 1, new_dir_file);
        fclose(new_dir_file);
        free(temp_name);
        
    }

}

void make_file(char *inodes, size_t *inodes_length, char *curr_directory, char *new_file) {
    char name[34] = "";
    uint32_t inode = 0;
    if (*inodes_length < 1023) {
        ++(*inodes_length);
        uint32_t new_inode = *inodes_length - 1;
        uint32_t *new_inode_ptr = &new_inode;
        inodes[new_inode] = 'f';
        //open the curr dir and write to it 
        //trunc name length to 32 chars
        char truncated_name[32];

        int i = 0;
        if (strlen(new_file) > 32) {
            for (i = 0; i < 32; i++) {
                truncated_name[i] = new_file[i];
            }
        }
        else {
            for (i = 0; i < strlen(new_file) - 1; i++) {
                truncated_name[i] = new_file[i];
            }
                truncated_name[i] = '\0';
        }


        //check if file name already exists in curr directory
        FILE *directory_file = open_file(curr_directory, "a+");
        while (fread(&inode, sizeof(int), 1, directory_file) == 1) {
            if (fread(&name, 32, 1, directory_file) == 1) {
            }
            strcat(name, "\n");
            if (strcmp(new_file, name) == 0 && inodes[inode] == 'f') {
                printf("file with that name already exists\n");
                return;
            }
        }

        //update contents of current directory
        fwrite(new_inode_ptr, 4, 1, directory_file);
        fwrite(truncated_name, 32, 1, directory_file);
        fclose(directory_file);

        //create real corresponding file
        char *temp_name = uint32_to_str(new_inode);
        FILE *new_reg_file = open_file(temp_name, "w");
        fclose(new_reg_file);
        free(temp_name);
    }
    
}

void update_inodes_file(char *inodes, size_t *inodes_length) {
    int i = 0;
    FILE *inodes_file = open_file("inodes_list", "w");
    uint32_t new_inode = 0;
    uint32_t *new_inode_ptr = &new_inode;
    char new_type = 'f';
    char *new_type_ptr = &new_type;

    
    for (i = 0; i < *inodes_length; i++) {
        new_inode = i;
        new_type = inodes[i];
        fwrite(new_inode_ptr, 4, 1, inodes_file);
        fwrite(new_type_ptr, 1, 1, inodes_file);
    }
    fclose(inodes_file);
    
}

void handle_commands(FILE *in, char *inodes, size_t *inodes_length) {
    char *line = NULL;
    size_t size;
    char *word = NULL; 
    char new_directory[33];
    char new_file[33];
    char curr_directory[5] = "0";


    while(getline(&line, &size, in) > 0) { 

        word = strtok(line, " ");
        if (strcmp(line, "exit\n") == 0) {
            free(line);
            update_inodes_file(inodes, inodes_length);
            exit(1);
        }
        else if (strcmp(word, "cd") == 0) {
            word = strtok(NULL, " ");
            strcpy(new_directory, word);
            update_directory(inodes, curr_directory, new_directory);
            
        }
        else if (strcmp(line, "ls\n") == 0) {
            list_contents(curr_directory);
        }

        else if (strcmp(word, "mkdir") == 0) {
            word = strtok(NULL, " ");
            strcpy(new_directory, word);
            make_directory(inodes, inodes_length, curr_directory, new_directory);
        }

        else if (strcmp(word, "touch") == 0) {
            word = strtok(NULL, " ");
            strcpy(new_file, word);
            make_file(inodes, inodes_length, curr_directory, new_file);
        }

        else {
            printf("invalid command\n");
        }
    }
    free(line);

}


void create_inode_list(char *inodes, size_t *inodes_length) {
    uint32_t inode = 0;
    int type = 0;
    FILE *inodes_list = open_file("inodes_list", "r");
    while (fread(&inode, sizeof(int), 1, inodes_list) == 1) {
        if (fread(&type, sizeof(char), 1, inodes_list) == 1) {
        }

        //check for invalid inodes or indicators
        if ((inode >= 0 && inode <= 1023) && ((type == 'd') || (type == 'f'))) {
            inodes[inode] = type;
            // inodes_length++;
            ++(*inodes_length);
        }
    }
    fclose(inodes_list);
}

int main(int argc, char *argv[]) {
    char inodes[INODE_COUNT]; // 1024  [ ["32bit", "tpye"] ]
    size_t inodes_length = 0;
    chdir(argv[FILE_IDX]);
    create_inode_list(inodes, &inodes_length);
    FILE *in = stdin;
    handle_commands(in, inodes, &inodes_length);
    fclose(in);

    return 0;
}

// hexdump -c fs/inodes_list
// hsearch cd 
// valgrind --leak-check=yes ./fs_emulator fs
//leaks on mac