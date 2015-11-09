#include "sfs_api.h"
#include "disk_emu.h"
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define DISK_FILE "sfs_disk.disk"
#define BLOCK_SIZE 512
#define MAX_BLOCKS 100

#define MAX_INODES 5
#define MAX_FILES MAX_INODES

super_block_t sb;
dir_entry_t root_dir[MAX_INODES];

inode_t inode_table[MAX_INODES];
fd_table_t fd_table[MAX_FILES];

unsigned short free_blocks[MAX_BLOCKS];

char file_data[] = "abcdefghijklmnopqrstuvwxyz";


int get_free_inode() {

    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_table[i].link_cnt == 0) {
            return i;
        }
    }
    return -1;
}

int get_existing_inode(char *name){
    for
    return -1;

}

int get_free_filedescriptor() {
    for (int i = 3; i < MAX_FILES; i++) {
        if (fd_table[i].inode_idx == -1) {
            return i;
        }

    }
    return -1;
}

void init_superblock() {

    // init the superblock
    sb.magic = 1234;
    sb.block_size = BLOCK_SIZE;
    sb.fs_size = MAX_BLOCKS * BLOCK_SIZE;
    sb.inode_table_len = MAX_INODES;
    sb.root_dir_inode = 0;
}

void add_root_dir_inode() {

    //first entry in the inode table is the root
    inode_table[0].mode = 0x755;
    inode_table[0].link_cnt = 1;
    inode_table[0].uid = 0;
    inode_table[0].gid = 0;
    inode_table[0].size = 45;
    inode_table[0].data_ptrs[0] = 2; //root dir is stored in the 3rd block


}

void add_dummy_file_inode() {

    //second entry is the dummy file
    inode_table[1].mode = 0x755;
    inode_table[1].link_cnt = 1;
    inode_table[1].uid = 0;
    inode_table[1].gid = 0;
    inode_table[1].size = strlen(file_data);
    inode_table[1].data_ptrs[0] = 3; //dummy file data is stored in the 4th block

}

void add_new_inode(int inode_index, unsigned int mode, unsigned int free_block) {

    //second entry is the dummy file
    inode_table[inode_index].mode = mode;
    inode_table[inode_index].link_cnt = 1;
    inode_table[inode_index].uid = 0;
    inode_table[inode_index].gid = 0;
    inode_table[inode_index].size = 0;
    inode_table[inode_index].data_ptrs[0] = free_block; //dummy file data is stored in the 4th block

}

void add_dummy_file_dir_entry() {

    root_dir[0].inode_idx = 1;
    strcpy(root_dir[0].name, "dummyfile.txt");

}

void add_new_file_dir_entry(unsigned int inode_index, char name[]) {

    root_dir[0].inode_idx = inode_index;
    strcpy(root_dir[0].name, name);

}

void sync_sfs() {
    printf("Writing inode table\n");
    add_root_dir_inode();
    add_dummy_file_inode();
    write_blocks(INODE_TABLE_BLOCK, 1, &inode_table);

    // write root directory data to the 3rd block
    printf("Writing root dir\n");
    add_dummy_file_dir_entry();

    write_blocks(DIRECTORY_TABLE_BLOCK, 1, &root_dir);
    write_blocks(MAX_BLOCKS - 1, 1, &free_blocks);
}

void add_dummy_file_data() {

    write_blocks(3, 1, &file_data);
}

void zero_everything() {

    bzero(&sb, sizeof(super_block_t));
    bzero(&fd_table[0], sizeof(fd_table_t) * MAX_FILES);
    bzero(&inode_table[0], sizeof(inode_t) * MAX_INODES);
    bzero(&root_dir, sizeof(dir_entry_t));
    bzero(&free_blocks[0], sizeof(unsigned int) * MAX_BLOCKS);

}

void mksfs(int fresh) {
    //Implement mksfs here
    if (fresh == 1) {

        //begin
        printf("Initalizing sfs\n");
        init_fresh_disk(DISK_FILE, BLOCK_SIZE, MAX_BLOCKS);
        zero_everything();


        // write superblock to the first block
        printf("Writing superblocks\n");
        init_superblock();
        write_blocks(0, 1, &sb);


        // write the inode table to the 2nd block
        printf("Writing inode table\n");
        add_root_dir_inode();
        add_dummy_file_inode();
        write_blocks(1, 1, &inode_table);

        // write root directory data to the 3rd block
        printf("Writing root dir\n");
        add_dummy_file_dir_entry();
        write_blocks(2, 1, &root_dir);

        // write file data to the 4th block
        printf("Writing file data\n");
        add_dummy_file_data();


        //mark blocks as used
        printf("Writing free blocks\n");
        free_blocks[0] = 1; //superblock
        free_blocks[1] = 1; //inode table
        free_blocks[2] = 1; //root dir data
        free_blocks[3] = 1; //dummy file data
        free_blocks[MAX_BLOCKS - 1] = 1; //free blocks

        // write the free blocks to the disk
        write_blocks(MAX_BLOCKS - 1, 1, &free_blocks);

    } else {

        init_disk(DISK_FILE, BLOCK_SIZE, MAX_BLOCKS);
        // pull back data from disk to mem
    }

    return;
}

int sfs_getnextfilename(char *fname) {

    static int seen = 0;

    printf("Calling sfs get next file\n");

    if (!seen) {

        seen = 1;
        strcpy(fname, root_dir[0].name);
        return 1;
    }

    seen = 0;
    return 0;
}


int sfs_getfilesize(const char *path) {

    //Implement sfs_getfilesize here
    int inode_idx = root_dir[0].inode_idx;
    inode_t inode = inode_table[inode_idx];

    return inode.size;
}

int check_file_exists(char *name) {
    for (int i = 0; i < MAX_INODES; i++) {
        if (root_dir[i].inode_idx == -1) continue;
        if (strcmp(root_dir[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}
int check_if_file_open(unsigned int inode_idx){
    for (int i = 3, i< MAX_FILES; i++){
        if (root_dir[i].inode_idx == inode_idx){
            return i;
        }
    }
    return -1;
}

int get_free_block() {
    for (int i = DIRECTORY_TABLE_BLOCK + 1; i < MAX_BLOCKS; i++) {
        if (free_blocks[i] == 0) {
            return i;
        }
    }
    return -1;
}

int sfs_fopen(char *name) {
    int available_inode = -1, available_block = -1;
    //Implement sfs_fopen here
    int fount_inode = check_file_exists(name);

    printf("Opening %s\n", name);

    if (fount_inode == -1) {
        available_inode = get_free_inode(), available_block = get_free_block();
        if (available_inode <0 || available_block < 0){
            return -2;
        }
        add_new_file_dir_entry((unsigned) available_inode,name);
        add_new_inode(available_inode,0x660,(unsigned)available_block);
        fount_inode = available_inode;
    }

    if (check_if_file_open((unsigned) fount_inode) == -1){
        int fd = get_free_filedescriptor();
        fd_table[fd].inode_idx = (unsigned) fount_inode;
        fd_table[fd].rd_write_ptr = 0;
        return fd;
    }
    return -1; //or fd already open


}

int sfs_fclose(int fileID) {

    //Implement sfs_fclose here
    fd_table[fileID].inode_idx = -1;
    fd_table[fileID].rd_write_ptr = 0;

    return 0;
}

int sfs_fread(int fileID, char *buf, int length) {

    //Implement sfs_fread here
     int inode_idx = fd_table[fileID].inode_idx;
    unsigned int cur_pos = fd_table[fileID].rd_write_ptr;
    char buffer[BLOCK_SIZE];

    inode_t file_inode = inode_table[inode_idx];

    //trying to read beyond the file
    if (cur_pos + length > file_inode.size) {
        length = file_inode.size - cur_pos;
    }

    if (length > 0) {

        //find how many bloks to read
        int block_idx = file_inode.data_ptrs[0];
        read_blocks(block_idx, 1, &buffer[0]);

        //check if length > block in that case read more

        memcpy(buf, buffer, length);
        return length;
    }

    return 0;
}

int sfs_fwrite(int fileID, const char *buf, int length) {


    //Implement sfs_fwrite here
    return 0;
}

int sfs_fseek(int fileID, int loc) {

    //should check if loc is a valid length

    fd_table[fileID].rd_write_ptr = loc;
    return 0;
}

int sfs_remove(char *file) {

    //clear the data blocks
    //clear the inodes
    //clear the dir entry
    //set 0 in free block map where the file used to be

    return 0;
}
