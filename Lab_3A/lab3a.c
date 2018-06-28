/* NAME: Derek Xu,Harry Yee
 * EMAIL: derekqxu@g.ucla.edu,hyee1234@gmail.com
 * ID: 704751588,704754172
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include "ext2_fs.h"

#define OFFSET 1024
#define BLOCK 1024

// general
int i, j, k, m;

// file system variables
char *fs_image_name;
int fs_image_fd;
int fs_info_fd;
/* reference for fs_image_fd:
 *                             +------- data structures -----------------------+
 *                             |                                               v
 * +-------+--------------+---------+---------+-------+-----------+       +-------------+
 * | super | blk grp desc | group 0 | group 1 | . . . | group n-1 | . . . | other stuff |
 * +-------+--------------+---------+---------+-------+-----------+       +-------------+
 *         |<----1024---->|
 */

// file system info variables
struct ext2_super_block super_ext2;
struct ext2_group_desc *group_ext2;
int group_num;
int block_size;
void format_time(uint32_t seconds, char * buf3)
{
    time_t s = seconds;
    struct tm *t = gmtime(&s);
    strftime(buf3,32,"%m/%d/%y %H:%M:%S", t);
}

// helper functions
void superblock_summary();
void group_summary();
void free_block_entries();
void free_I_node_entries();
void I_Node_summary();
void directory_entries();
void indirect_block_references(__u32 *offset, int level, int indir_block_offset);

int main(int argc, char *argv[])
{
    // option handling
    if (argc != 2){
        fprintf(stderr, "improper arguments!\n");
        exit(1);
    }
    fs_image_name = argv[1];

    // open the image file
    fs_image_fd = open(fs_image_name, O_RDONLY);
    if (fs_image_fd == -1){
        fprintf(stderr, "cannot open file!\n");
        exit(1);
    }

    // create CSV file to save info
    fs_info_fd = creat("fs_analysis.csv", S_IRWXU);

    // get super block_ext2 information
    pread(fs_image_fd, &super_ext2, sizeof( struct ext2_super_block ), OFFSET);
    // get block size
    block_size = BLOCK << super_ext2.s_log_block_size;
    // get # of groups
    group_num = super_ext2.s_blocks_count / super_ext2.s_blocks_per_group + 1;
    // get group_ext2 information
    group_ext2 = malloc(group_num * sizeof(struct ext2_group_desc));
    for (i = 0; i < group_num; ++i)
        pread(fs_image_fd, &group_ext2[i], sizeof(struct ext2_group_desc), 2*BLOCK + i*sizeof(struct ext2_group_desc));

    // fill out the CSV
    superblock_summary();
    group_summary();
    free_block_entries();
    free_I_node_entries();
    I_Node_summary();

    // close open files
    close(fs_image_fd);
    close(fs_info_fd);

    //print csv
    int data = open("fs_analysis.csv", O_RDONLY);
    char buf[2048];
    int read_size;
    do{
        read_size = read(data, buf, 1024);
        write(STDOUT_FILENO, buf, read_size);
    }
    while (read_size > 0);
    close(data);

    exit(0);
}

void superblock_summary()
{
    /* get following:
     * total # of blocks
     * total # of inodes
     * block size
     * inode size
     * blocks per group
     * inodes per group
     * first non-reserved inode
     */
    dprintf(fs_info_fd, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
            super_ext2.s_blocks_count,
            super_ext2.s_inodes_count,
            block_size,
            super_ext2.s_inode_size,
            super_ext2.s_blocks_per_group,
            super_ext2.s_inodes_per_group,
            super_ext2.s_first_ino);
}
void group_summary()
{
    // get remainder block information
    int block_remaining = super_ext2.s_blocks_count % super_ext2.s_blocks_per_group;
    int inode_remaining = super_ext2.s_inodes_count % super_ext2.s_inodes_per_group;

    /* get following:
     * group #
     * total # of blocks in group
     * total # of inodes in group
     * # of free blocks
     * # of free inodes
     * block # of free block bitmap
     * block # of free inode bitmap
     * block # of first block of inodes
     */
    for (i = 0; i < group_num; ++i){
        //pread(fs_image_fd, &group_ext2[i], sizeof(struct ext2_group_desc), 2*BLOCK + i*sizeof(struct ext2_group_desc));
        dprintf(fs_info_fd, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",
                i,
                (i == (group_num-1) && (block_remaining != 0))? block_remaining: (int) super_ext2.s_blocks_per_group,
                (i == (group_num-1) && (inode_remaining != 0))? inode_remaining: (int) super_ext2.s_inodes_per_group,
                group_ext2[i].bg_free_blocks_count,
                group_ext2[i].bg_free_inodes_count,
                group_ext2[i].bg_block_bitmap,
                group_ext2[i].bg_inode_bitmap,
                group_ext2[i].bg_inode_table);
    }
}
void free_block_entries()
{
    /* get following:
     * free blocks
     */
    // iterate through each bit of each byte in block bitmap
    char byte_buf;
    for (i = 0; i < group_num; ++i){
        for (j = 0; j < block_size; ++j){
            // get the block bitmap bytes at offset = bg_block_bitmap * block_size
            pread(fs_image_fd, &byte_buf, 1, (group_ext2[i].bg_block_bitmap * block_size) + j);
            for (k = 0; k < 8; ++k)
                if( (byte_buf & (0x1 << k)) == 0)
                    dprintf(fs_info_fd, "BFREE,%d\n", i*super_ext2.s_blocks_per_group + 8*j + k + 1);
        }
    }
}
void free_I_node_entries()
{
    /* get following:
     * free inodes
     */
    // iterate through each bit of each byte in inode bitmap
    char byte_buf;
    for (i = 0; i < group_num; ++i){
        for (j = 0; j < block_size; ++j){
            // get the inode bitmap bytes at offset = bg_block_bitmap * block_size
            pread(fs_image_fd, &byte_buf, 1, (group_ext2[i].bg_inode_bitmap * block_size) + j);
            for (k = 0; k < 8; ++k)
                if( (byte_buf & (0x1 << k)) == 0)
                    dprintf(fs_info_fd, "IFREE,%d\n", i*super_ext2.s_blocks_per_group + 8*j + k + 1);
        }
    }
}
void I_Node_summary()
{
    char byte_buf2; // we don't talk about byte_buf1... :(
    char time_buf2[256];
    char time_buf3[256];
    char time_buf4[256];
    struct ext2_inode inode_ext2;
    // iterate through each inode in the inode table of each group
    for (i = 0; i < group_num; ++i)
        for (j = 0; j < (int) super_ext2.s_inodes_per_group; ++j){
            pread(fs_image_fd, &inode_ext2, sizeof( struct ext2_inode ), (group_ext2[i].bg_inode_table * block_size) + (j * sizeof( struct ext2_inode )));
            // if the file is not valid skip
            if (inode_ext2.i_mode == 0 || inode_ext2.i_links_count == 0)
                continue;
            // get file type
            byte_buf2 = '?';
            if (inode_ext2.i_mode & 0x8000)
                byte_buf2 = 'f';
            else if (inode_ext2.i_mode & 0x4000)
                byte_buf2 = 'd';
            else if (inode_ext2.i_mode & 0xA000)
                byte_buf2 = 's';
            // format time
            format_time(inode_ext2.i_ctime, time_buf2);
            format_time(inode_ext2.i_mtime, time_buf3);
            format_time(inode_ext2.i_atime, time_buf4);
            /* get following:
             * inode #
             * file type
             * mode
             * owner
             * group
             * link count
             * time of last inode change
             * modification time
             * time of last access
             * file size
             * number of blocks of disk space
             */
            dprintf(fs_info_fd, "INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",
                    j + (super_ext2.s_inodes_per_group*i) + 1,
                    byte_buf2,
                    inode_ext2.i_mode & 0xFFF,
                    inode_ext2.i_uid,
                    inode_ext2.i_gid,
                    inode_ext2.i_links_count,
                    time_buf2,
                    time_buf3,
                    time_buf4,
                    inode_ext2.i_size,
                    inode_ext2.i_blocks
                    );
            /* get following:
             * block addresses x15
             */
            for (k = 0; k < EXT2_N_BLOCKS; ++k)
                dprintf(fs_info_fd, ",%d", inode_ext2.i_block[k]);
            dprintf(fs_info_fd, "\n");

            if (byte_buf2 == 'd')
                directory_entries(&inode_ext2);

            // spec says to only check files and directories
            if (byte_buf2 == 'f' || byte_buf2 == 'd'){
                // offsets found in the EXT2 manual
                indirect_block_references(&inode_ext2.i_block[12], 1, 12);
                indirect_block_references(&inode_ext2.i_block[13], 2, 268);
                indirect_block_references(&inode_ext2.i_block[14], 3, 65804);
            }
        }
}
void directory_entries(struct ext2_inode *inode_ext2)
{
    struct ext2_dir_entry dir_ext2;
    // NOTE: cannot use i or j variables
    // scan through each entry in each block
    for (k = 0; (k < EXT2_NDIR_BLOCKS) && (inode_ext2->i_block[k] != 0); ++k)
        for (m = 0; m < BLOCK; m += (int) dir_ext2.rec_len){
            pread(fs_image_fd, &dir_ext2, sizeof(struct ext2_dir_entry), inode_ext2->i_block[k] * BLOCK + m);
            if (dir_ext2.inode == 0)
                continue;
            /* get following:
             * parent inode number
             * logical byte offset
             * inode number of referenced file
             * entry length
             * name length
             * name
             */
            dprintf(fs_info_fd,"DIRENT,%d,%d,%d,%d,%d,\'%s\'\n",
                    j + (super_ext2.s_inodes_per_group*i) + 1,
                    m,
                    dir_ext2.inode,
                    dir_ext2.rec_len,
                    dir_ext2.name_len,
                    dir_ext2.name);
        }
}
void indirect_block_references(__u32 *offset, int level, int indir_block_offset)
{
    if (level == 0 || *offset == 0)
        return;
    __u32 *indir_block = malloc(BLOCK);
    pread(fs_image_fd, indir_block, BLOCK, *offset * BLOCK);
    // NOTE: cannot use i or j variables
    // scan through each pointer
    int n;
    for (n = 0; n < BLOCK/(int) sizeof(__u32); ++n){
        /* get following:
         * inode number of owning file
         * level of indirection
         * logical block offset of REFERENCED BLOCK
         * block number of CURRENT BLOCK
         * block number of the REFERENCED BLOCK
         */
        if (indir_block[n] != 0){
            dprintf(fs_info_fd,"INDIRECT,%d,%d,%d,%d,%d\n",
                    j + (super_ext2.s_inodes_per_group*i) + 1,
                    level,
                    n + indir_block_offset,
                    *offset,
                    indir_block[n]);
            indirect_block_references(&indir_block[n], level-1, indir_block_offset);
        }
    }
}
