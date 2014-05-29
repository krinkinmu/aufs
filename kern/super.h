#ifndef __SUPER_H__
#define __SUPER_H__

#include <linux/types.h>

struct aufs_super_block
{
	uint32_t	asb_magic;
	uint32_t	asb_inode_blocks;
	uint32_t	asb_block_size;
	uint32_t	asb_root_inode;
	uint32_t	asb_inodes_in_block;
};

static inline struct aufs_super_block *AUFS_SB(struct super_block *sb)
{
	return (struct aufs_super_block *)sb->s_fs_info;
}

#endif /*__SUPER_H__*/
