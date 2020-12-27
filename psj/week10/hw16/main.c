#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct super_block {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t r_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mnt_count;
    uint16_t max_mnt_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;
    uint32_t first_inode;
    uint16_t inode_size;
    // uint16_t block_group_nr;
    // uint32_t feature_compat;
    // uint32_t feature_incompat;
    // uint32_t feature_ro_compat;
    // uint8_t uuid[16];
    // char volume_name[16];
    // char last_mounted[64];
    // uint32_t algorithusage_bitmap;
    // uint8_t prealloc_blocks;
    // uint8_t prealloc_dir_blocks;
    // uint16_t padding;
    // uint8_t journal_uuid[16];
    // uint32_t journal_inum;
    // uint32_t journal_dev;
    // uint32_t last_orphan;
    // uint32_t hash_seed[4];
};

struct group_desc {
    uint32_t block_bitmap; // block location of DBM
    uint32_t inode_bitmap; // block location of IBM
    uint32_t inode_table;  // block location of inode table
    // uint16_t free_blocks_count;
    // uint16_t free_inodes_count;
    // uint16_t used_dir_count;
    // uint16_t padding;
    // uint32_t reserved[3];
};

struct inode {
    uint16_t mode;
    uint16_t uid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t reserved1;
    uint32_t block[15]; // block location of this file
    // uint32_t generation;
    // uint32_t file_acl;
    // uint32_t dir_acl;
    // uint32_t faddr;
    // uint32_t reserved2[3];
};

struct dir_entry {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[255];
};

int main(void) {
    int fd = open("myfd", O_RDONLY);

    size_t first_data_block = 1;
    size_t block_size = 1024;

    size_t super_block_idx = block_size * first_data_block; // 2nd block
    lseek(fd, super_block_idx, SEEK_SET);
    struct super_block *super_block = (struct super_block *) malloc(sizeof(struct super_block));
    read(fd, super_block, sizeof(struct super_block));

    assert(first_data_block == super_block->first_data_block);
    assert(block_size == block_size * pow(2, super_block->log_block_size));

    printf("<Superblock>\n");
    printf("inodes_count = %d\n", super_block->inodes_count);
    printf("blocks_count = %d\n", super_block->blocks_count);
    printf("free_blocks_count = %d\n", super_block->free_blocks_count);
    printf("free_inodes_count = %d\n", super_block->free_inodes_count);
    printf("first_data_block = %d\n", first_data_block);
    printf("log_block_size = %d\n", super_block->log_block_size);
    printf("block_size = %d\n", block_size);
    printf("inode_size = %d\n", super_block->inode_size);
    printf("\n");

    size_t group_desc_idx = block_size * (first_data_block + 1); // 3rd block
    lseek(fd, group_desc_idx, SEEK_SET);
    struct group_desc *group_desc = (struct group_desc *) malloc(sizeof(struct group_desc));
    read(fd, group_desc, sizeof(struct group_desc));

    printf("<Group Descriptor>\n");
    printf("block location of DBM = %d\n", group_desc->block_bitmap);
    printf("block location of IBM = %d\n", group_desc->inode_bitmap);
    printf("block location of inode table = %d\n", group_desc->inode_table);
    printf("\n");

    size_t inode_table_idx = block_size * group_desc->inode_table + super_block->inode_size; // 2nd block of inode
    lseek(fd, inode_table_idx, SEEK_SET);
    struct inode *inode_table = (struct inode *) malloc(sizeof(struct inode));
    read(fd, inode_table, sizeof(struct inode));

    printf("<inode Table>\n");
    printf("block location of root file directory = %d\n", inode_table->block[0]);
    printf("\n");

    size_t root_idx = block_size * inode_table->block[0];
    lseek(fd, root_idx, SEEK_SET);
    struct dir_entry *dir = (struct dir_entry *) malloc(sizeof(struct dir_entry));
    read(fd, dir, sizeof(struct dir_entry));

    printf("<Root Directory>\n");
    for (int i = 0; i < block_size; i += dir->rec_len) {
        lseek(fd, root_idx + i, SEEK_SET);
        read(fd, dir, sizeof(struct dir_entry));
        printf("filename = %s\n", dir->name);
        printf("inode = %d\n", dir->inode);

        lseek(fd, block_size * group_desc->inode_table + super_block->inode_size * (dir->inode - 1), SEEK_SET);
        struct inode *inode = (struct inode *) malloc(sizeof(struct inode));
        read(fd, inode, sizeof(struct inode));
        printf("block location = %d\n", inode->block[0]);
        printf("\n");
        free(inode);
    }

    free(dir);
    free(inode_table);
    free(group_desc);
    free(super_block);
    close(fd);
    return 0;
}
