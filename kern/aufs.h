#ifndef __AUFS_H__
#define __AUFS_H__

#include <linux/types.h>
#include <linux/fs.h>

#define AUFS_DDE_SIZE         32
#define AUFS_DDE_MAX_NAME_LEN (AUFS_DDE_SIZE - sizeof(__be32))

static const unsigned long AUFS_MAGIC = 0x13131313;

struct aufs_disk_super_block {
	__be32	dsb_magic;
	__be32	dsb_block_size;
	__be32	dsb_root_inode;
	__be32	dsb_inode_blocks;
};

struct aufs_disk_inode {
	__be32	di_first;
	__be32	di_blocks;
	__be32	di_size;
	__be32	di_gid;
	__be32	di_uid;
	__be32	di_mode;
	__be64	di_ctime;
};

struct aufs_disk_dir_entry {
	char dde_name[AUFS_DDE_MAX_NAME_LEN];
	__be32 dde_inode;
};

struct aufs_super_block {
	unsigned long asb_magic;
	unsigned long asb_inode_blocks;
	unsigned long asb_block_size;
	unsigned long asb_root_inode;
	unsigned long asb_inodes_in_block;
};

static inline struct aufs_super_block *AUFS_SB(struct super_block *sb)
{
	return (struct aufs_super_block *)sb->s_fs_info;
}

struct aufs_inode {
	struct inode ai_inode;
	unsigned long ai_block;
};

static inline struct aufs_inode *AUFS_INODE(struct inode *inode)
{
	return (struct aufs_inode *)inode;
}

extern const struct address_space_operations aufs_aops;
extern const struct inode_operations aufs_dir_inode_ops;
extern const struct file_operations aufs_file_ops;
extern const struct file_operations aufs_dir_ops;

struct inode *aufs_inode_get(struct super_block *sb, unsigned long no);
struct inode *aufs_inode_alloc(struct super_block *sb);
void aufs_inode_free(struct inode *inode);

#endif /*__AUFS_H__*/
