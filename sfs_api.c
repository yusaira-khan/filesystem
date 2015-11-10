#include "sfs_api.h"
#include "disk_emu.h"
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#define DISK_FILE "sfs_disk.disk"
#define BLOCK_SIZE 512
#define MAX_BLOCKS 100

#define MAX_INODES 5
#define MAX_FILES MAX_INODES

super_block_t sb;
dir_entry_t root_dir[MAX_INODES];

inode_t inode_table[MAX_INODES];
fd_table_t fd_table[MAX_FILES];

unsigned short all_blocks[MAX_BLOCKS];


unsigned int get_free_inode() {

    for (unsigned int i = FIRST_AVAILABLE_INODE; i < MAX_INODES; i++) {
        if (inode_table[i].link_cnt == 0) {
            return i;
        }
    }
    return UNAVAILABLE_INODE;
}

int get_free_filedescriptor() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!fd_table[i].inode_idx) {
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
    sb.root_dir_inode = ROOT_INODE;
}

void add_root_dir_inode() {

    //first entry in the inode table is the root
    inode_table[0].mode = 0x755;
    inode_table[0].link_cnt = 1;
    inode_table[0].uid = 0;
    inode_table[0].gid = 0;
    inode_table[0].size = 45;
    inode_table[0].data_ptrs[0] = DIRECTORY_TABLE_BLOCK; //root dir is stored in the 3rd block
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

int get_unused_directory_spot() {
    for (int i = 0; i < MAX_INODES; i++) {
        if (!root_dir[i].inode_idx) {
            return i;
        }
    }
    return -1;
}

void add_new_file_dir_entry(unsigned int inode_index, char name[]) {
    int idx = get_unused_directory_spot();
    root_dir[idx].inode_idx = inode_index;
    strcpy(root_dir[idx].name, name);
}

void sync_sfs() {
    printf("Writing inode table\n");
    add_root_dir_inode();
    write_blocks(INODE_TABLE_BLOCK, 1, &inode_table);

    // write root directory data to the 3rd block
    printf("Writing root dir\n");

    write_blocks(DIRECTORY_TABLE_BLOCK, 1, &root_dir);
    write_blocks(MAX_BLOCKS - 1, 1, &all_blocks);
}


void zero_everything() {

    bzero(&sb, sizeof(super_block_t));
    bzero(&fd_table[0], sizeof(fd_table_t) * MAX_FILES);
    bzero(&inode_table[0], sizeof(inode_t) * MAX_INODES);
    bzero(&root_dir, sizeof(dir_entry_t));
    bzero(&all_blocks[0], sizeof(unsigned int) * MAX_BLOCKS);

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
        write_blocks(1, 1, &inode_table);

        // write root directory data to the 3rd block
        printf("Writing root dir\n");
        write_blocks(2, 1, &root_dir);

        //mark blocks as used
        printf("Writing free blocks\n");
        all_blocks[SUPERBLOCK] = USED; //superblock
        all_blocks[INODE_TABLE_BLOCK] = USED; //inode table
        all_blocks[DIRECTORY_TABLE_BLOCK] = USED; //root dir data
        all_blocks[MAX_BLOCKS - 1] = USED; //free blocks

        // write the free blocks to the disk
        write_blocks(MAX_BLOCKS - 1, 1, &all_blocks);

    } else {

        init_disk(DISK_FILE, BLOCK_SIZE, MAX_BLOCKS);
        // pull back data from disk to mem
    }
}

int sfs_getnextfilename(char *fname) {

    static int current_file_ptr = 0;

    printf("Calling sfs get next file\n");

    for (int looper = 0; looper < MAX_FILES; looper++) {
        current_file_ptr = (current_file_ptr + 1) % MAX_FILES;
        if (root_dir[current_file_ptr].inode_idx) {
            strcpy(fname, root_dir[0].name);
            return 1;
        }
    }
    current_file_ptr = 0; //No file in directory
    return 0;
}

unsigned int get_directory_ptr_from_name(const char *name) {
    for (unsigned int i = FIRST_AVAILABLE_INODE; i < MAX_INODES; i++) {
        if (!root_dir[i].inode_idx) continue;
        if (strcmp(root_dir[i].name, name) == 0) {
            return i;
        }
    }
    return UNAVAILABLE_INODE;
}

int sfs_getfilesize(const char *path) {

    //Implement sfs_getfilesize here

    int directory_ptr = get_directory_ptr_from_name(path);
    int unsigned inode_idx;
    inode_idx = root_dir[directory_ptr].inode_idx;
    inode_t inode = inode_table[inode_idx];
    return inode.size;
}


int check_if_file_open(int inode_idx) {
    for (int i = 3; i < MAX_FILES; i++) {
        if (root_dir[i].inode_idx == inode_idx) {
            return i;
        }
    }
    return -1;
}

unsigned int get_free_block() {
    for (unsigned int i = FIRST_AVAILABLE_BLOCK; i < MAX_BLOCKS; i++) {
        if (all_blocks[i] == 0) {
            return i;
        }
    }
    return UNAVAILABLE_BLOCK;
}


int sfs_fopen(char *name) {
    //Implement sfs_fopen here
    int directory_ptr = get_directory_ptr_from_name(name);
    unsigned int fount_inode = root_dir[directory_ptr].inode_idx;
    int fd;

    if (!fount_inode) {
        unsigned int available_inode = get_free_inode(), available_block = get_free_block();
        if (!available_block || !available_inode) {
            fprintf(stderr, "No space to open file! All blocks or Inodes occupied.");
            return -2;
        }
        //TODO: check max 16 char for name + . + 3 char for ext
        add_new_file_dir_entry(available_inode, name);
        add_new_inode(available_inode, 0x660, available_block);
        fount_inode = available_inode;
        fd = -1;
    } else {
        fd = check_if_file_open(fount_inode);
    }

    if (fd == -1) {
        fd = get_free_filedescriptor();
        printf("Opening %s fd:%d, inode:%u\n", name, fd, fount_inode);
        if (fd == -1) {
            fprintf(stderr, ("cannot open anymore files"));
            return -3;
        }
        fd_table[fd].inode_idx = fount_inode;
        fd_table[fd].rd_write_ptr = 0;
        return fd;
    }
    return fd; //already open
}

int sfs_fclose(int fileID) {

    //Implement sfs_fclose here
    fd_table[fileID].inode_idx = UNAVAILABLE_INODE;
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

    if (length <= 0) return length;

    int num_used_pts = length / BLOCK_SIZE, incomplete_block = length % BLOCK_SIZE,
            cur_ptr = cur_pos / BLOCK_SIZE, pos_in_block= cur_pos%BLOCK_SIZE,
            last_used_ptr = file_inode.size/BLOCK_SIZE + pos_in_block ? 1 : 0,
            bytes_used = 0,  read_length = 0;
    num_used_pts += pos_in_block ? 1 :0;
    assert(num_used_pts <= last_used_ptr); //lenght is not greater than file size
    assert(last_used_ptr < MAX_DIRECT_DATA); //size is not greater than current limit
    for (int block_idx =0 ; cur_ptr <= num_used_pts && cur_ptr < MAX_DIRECT_DATA;
           cur_ptr++, bytes_used += read_length, pos_in_block = 0) {

        block_idx = file_inode.data_ptrs[cur_ptr];
        assert(block_idx); //block_idx cannot be empty since it's less than file size

        if (cur_ptr == num_used_pts){
            read_length = BLOCK_SIZE - pos_in_block;
        }else{
            read_length = incomplete_block - pos_in_block;
        }
        read_blocks(block_idx, 1, &buffer[0]);
        memcpy(buf + bytes_used, buffer+pos_in_block, read_length);
    }

    fd_table[fileID].rd_write_ptr += (unsigned) read_length;
    return read_length;


}

int sfs_fwrite(int fileID, const char *buf, int length) {
    int inode_idx = fd_table[fileID].inode_idx;
    unsigned int cur_pos = fd_table[fileID].rd_write_ptr;
    char buffer[BLOCK_SIZE];

    inode_t file_inode = inode_table[inode_idx];

    //trying to read beyond the file
    if (cur_pos + length > file_inode.size) {
        length = file_inode.size - cur_pos;
    }

    if (length <= 0) return length;

    int num_used_ptrs = length / BLOCK_SIZE, incomplete_block = length % BLOCK_SIZE,
            cur_ptr = cur_pos / BLOCK_SIZE, pos_in_block= cur_pos%BLOCK_SIZE,
            last_used_ptrs = file_inode.size/BLOCK_SIZE + pos_in_block ? 1 : 0,
            bytes_used = 0,  read_length = 0;
    num_used_ptrs += pos_in_block ? 1 :0;
    assert(num_used_ptrs <= MAX_DIRECT_DATA);
    assert(last_used_ptrs < MAX_DIRECT_DATA);
    for (int block_idx=0; cur_ptr <= num_used_ptrs && cur_ptr < MAX_DIRECT_DATA;
           cur_ptr++, bytes_used += read_length, pos_in_block = 0) {

         block_idx = file_inode.data_ptrs[cur_ptr];
        if (!block_idx){
            block_idx = get_free_block();
            if (!block_idx) {
                fprintf(stderr, "Disk Full! Failed to write %d blocks.\n",length- read_length);
                return read_length;
            }
            all_blocks[block_idx] = USED;

        }

        if (cur_ptr == num_used_ptrs){
            read_length = BLOCK_SIZE - pos_in_block;
        }else{
            read_length = incomplete_block - pos_in_block;
        }
        write_blocks(block_idx, 1, &buffer[0]);
        memcpy(buffer+pos_in_block,buf + bytes_used,  read_length);
        fd_table[fileID].rd_write_ptr += (unsigned) read_length;
        file_inode.size += cur_pos + read_length;
    }
    for (int unused_ptr = num_used_ptrs +1; unused_ptr <= last_used_ptrs; unused_ptr++){
        int block_index = file_inode.data_ptrs[unused_ptr];
        all_blocks[block_index] = FREE;
    }
    return read_length;


}

int sfs_fseek(int fileID, int loc) {

    //should check if loc is a valid length
    if (loc < 0) return  -1;
    inode_t i = inode_table[fd_table[fileID].inode_idx];
    if (loc> i.size) return -2;
    if (loc % BLOCK_SIZE > MAX_DIRECT_DATA) return -3;

    fd_table[fileID].rd_write_ptr = (unsigned) loc;
    return 0;
}

int sfs_remove(char *file) {
    const char *path = file;
    int directory_ptr = get_directory_ptr_from_name(path);
    if (directory_ptr == -1) {
        fprintf(stderr, "Cannot remove file '%s'. File Does Not Exist", file);
        return -1;
    }

    unsigned int inode_idx = root_dir[directory_ptr].inode_idx, block;
    inode_t cur_inode = inode_table[inode_idx];

    root_dir[directory_ptr].inode_idx = UNAVAILABLE_INODE;
    //clear the dir entry

    //clear the data blocks
    //set 0 in free block map where the file used to be
    for (int i = 0; i < MAX_DIRECT_DATA; i++) {
        block = cur_inode.data_ptrs[i];
        if (block) {
            cur_inode.data_ptrs[i] = UNAVAILABLE_BLOCK;
            all_blocks[block] = FREE;
        }
        else break; //All inodes arranged serially
    }
    //Do the same for indirect ptrs
    if (cur_inode.indirect_ptr) {

        all_blocks[cur_inode.indirect_ptr] = 0;
//        for(int i = 0; i<MAX_DATA_PER_INDIRECT; i++){
//            block =  cur_inode.data_ptrs[i];
//            if (block){
//                cur_inode.data_ptrs[i] = UNAVAILABLE_BLOCK;
//                free_blocks[block] = 0;
//            }
//            else break; //All inodes arranged serially
//        }
        cur_inode.indirect_ptr = 0;
    }
    cur_inode.size = 0;
    cur_inode.link_cnt = 0;
    cur_inode.mode = 0;


    return 0;
}
