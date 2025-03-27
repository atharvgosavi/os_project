#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ext2_fs.h"


void init_superblock(struct ext2_super_block *sb, uint32_t BCnt) {
    memset(sb, 0, sizeof(struct ext2_super_block));
    
    sb->s_inodes_count = EXT2_INODES_PER_GROUP;
    sb->s_blocks_count = BCnt;
    sb->s_r_blocks_count = BCnt / 20;  
    sb->s_free_blocks_count = BCnt - 1; 
    sb->s_free_inodes_count = EXT2_INODES_PER_GROUP - 1; 
    sb->s_first_data_block = EXT2_FIRST_DATA_BLOCK;
    sb->s_log_block_size = 0; 
    sb->s_log_frag_size = 0;
    sb->s_blocks_per_group = EXT2_BLOCKS_PER_GROUP;
    sb->s_frags_per_group = EXT2_BLOCKS_PER_GROUP;
    sb->s_inodes_per_group = EXT2_INODES_PER_GROUP;
    sb->s_mtime = 0; 
    sb->s_wtime = time(NULL); 
    sb->s_mnt_count = 0;
    sb->s_max_mnt_count = 65535;
    sb->s_magic = EXT2_SUPER_MAGIC;
    sb->s_state = 1; 
    sb->s_errors = 1; 
    sb->s_minor_rev_level = 0;
    sb->s_lastcheck = time(NULL);
    sb->s_checkinterval = 86400 * 180; 
    sb->s_creator_os = 0; 
    sb->s_rev_level = 0; 
    sb->s_def_resuid = 0;
    sb->s_def_resgid = 0;
}

void init_group_desc(struct ext2_group_desc *gd) {
    memset(gd, 0, sizeof(struct ext2_group_desc));
    
    gd->bg_block_bitmap = 2; 
    gd->bg_inode_bitmap = 3; 
    gd->bg_inode_table = 4; 
    gd->bg_free_blocks_count = EXT2_BLOCKS_PER_GROUP - 4; 
    gd->bg_free_inodes_count = EXT2_INODES_PER_GROUP - 1; 
    gd->bg_used_dirs_count = 1; 
    gd->bg_pad = 0;
}

void write_block_bitmap(int fd) {
    unsigned char *bitmap = calloc(EXT2_BLOCK_SIZE, 1);
    
    bitmap[0] = 0x0F;
    
    write(fd, bitmap, EXT2_BLOCK_SIZE);
    free(bitmap);
}

void write_inode_bitmap(int fd) {
    unsigned char *bitmap = calloc(EXT2_BLOCK_SIZE, 1);
    
    bitmap[0] = 0x01;
    
    write(fd, bitmap, EXT2_BLOCK_SIZE);
    free(bitmap);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <device>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    
    off_t device_size = lseek(fd, 0, SEEK_END);
    if (device_size < 0) {
        perror("Failed to get device size");
        close(fd);
        return 1;
    }

    uint32_t BCnt = device_size / EXT2_BLOCK_SIZE;


    struct ext2_super_block sb;
    init_superblock(&sb, BCnt);
    
    lseek(fd, EXT2_BLOCK_SIZE, SEEK_SET);
    write(fd, &sb, sizeof(sb));

    
    struct ext2_group_desc gd;
    init_group_desc(&gd);
    
    lseek(fd, EXT2_BLOCK_SIZE * 2, SEEK_SET);
    write(fd, &gd, sizeof(gd));

    
    lseek(fd, EXT2_BLOCK_SIZE * 2, SEEK_SET);
    write_block_bitmap(fd);

    
    lseek(fd, EXT2_BLOCK_SIZE * 3, SEEK_SET);
    write_inode_bitmap(fd);

    close(fd);
    printf("Successfully formatted device %s with ext2 filesystem\n", argv[1]);
    return 0;
}
