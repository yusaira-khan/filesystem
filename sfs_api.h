
#define MAXFILENAME 16
#define EXT_SIZE 3
#define SEP '.'

#define SUPERBLOCK 0
#define UNAVAILABLE_BLOCK SUPERBLOCK
#define INODE_TABLE_BLOCK 1
#define DIRECTORY_TABLE_BLOCK 2
#define FIRST_AVAILABLE_BLOCK 3

#define ROOT_INODE 0
#define UNAVAILABLE_INODE ROOT_INODE
#define FIRST_AVAILABLE_INODE 1

#define MAX_DIRECT_DATA 10
#define MAX_DATA_PER_INDIRECT MAX_DIRECT_DATA

#define FREE 0
#define USED 1

void mksfs(int fresh);
int sfs_getnextfilename(char *fname);
int sfs_getfilesize(const char* path);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_fread(int fileID, char *buf, int length);
int sfs_fwrite(int fileID, const char *buf, int length);
int sfs_fseek(int fileID, int loc);
int sfs_remove(char *file);


typedef struct super_block {
	unsigned int magic;
	unsigned int block_size;
	unsigned int fs_size;
	unsigned int inode_table_len;
	unsigned int root_dir_inode;
} super_block_t;


typedef struct inode { 
	unsigned int mode;
	unsigned int link_cnt;
	unsigned int uid;
	unsigned int gid;
	unsigned int size;
	unsigned int data_ptrs[MAX_DIRECT_DATA];
    unsigned int indirect_ptr;
} inode_t;

typedef struct indirect_data{
    unsigned int data_ptrs[MAX_DATA_PER_INDIRECT];
} indirect_t;

typedef struct dir_entry { 
	char name[MAXFILENAME];
	unsigned int inode_idx;
} dir_entry_t;


typedef struct fd_table { 
	unsigned int inode_idx;
	unsigned int rd_write_ptr;
} fd_table_t;





