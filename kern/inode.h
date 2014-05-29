#ifndef __INODE_H__
#define __INODE_H__

#include <linux/fs.h>

struct aufs_inode
{
	struct inode	ai_inode;
	uint32_t	ai_block;
};

static inline struct aufs_inode *AUFS_INODE(struct inode *inode)
{
	return (struct aufs_inode *)inode;
}

struct inode *aufs_inode_get(struct super_block *sb, uint32_t no);
struct inode *aufs_inode_alloc(struct super_block *sb);
void aufs_inode_free(struct inode *inode);

int aufs_inode_cache_create(void);
void aufs_inode_cache_destroy(void);

#endif /*__INODE_H__*/
