
#define MAXFILENAME 16

#define INODE_TABLE_BLOCK 1

#define DIRECTORY_TABLE_BLOCK 2

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
	unsigned int data_ptrs[10];
} inode_t;


typedef struct dir_entry { 
	char name[MAXFILENAME];
	int inode_idx;
} dir_entry_t;


typedef struct fd_table { 
	int inode_idx;
	unsigned int rd_write_ptr;
} fd_table_t;





