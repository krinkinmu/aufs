#include <linux/aio.h>
#include <linux/buffer_head.h>
#include <linux/mpage.h>
#include <linux/slab.h>

#include "aufs.h"

static void aufs_inode_fill(struct aufs_inode *ai,
			struct aufs_disk_inode const *di)
{
	ai->ai_block = be32_to_cpu(di->di_first);
	ai->ai_inode.i_mode = be32_to_cpu(di->di_mode);
	ai->ai_inode.i_size = be32_to_cpu(di->di_size);
	ai->ai_inode.i_blocks = be32_to_cpu(di->di_blocks);
	ai->ai_inode.i_ctime.tv_sec = be64_to_cpu(di->di_ctime);
	ai->ai_inode.i_mtime.tv_sec = ai->ai_inode.i_atime.tv_sec =
				ai->ai_inode.i_ctime.tv_sec;
	ai->ai_inode.i_mtime.tv_nsec = ai->ai_inode.i_atime.tv_nsec =
				ai->ai_inode.i_ctime.tv_nsec = 0;
	i_uid_write(&ai->ai_inode, (uid_t)be32_to_cpu(di->di_uid));
	i_gid_write(&ai->ai_inode, (gid_t)be32_to_cpu(di->di_gid));
}

static inline sector_t aufs_inode_block(struct aufs_super_block const *asb,
			ino_t inode_no)
{
	return (sector_t)(3 + inode_no / asb->asb_inodes_in_block);
}

static size_t aufs_inode_offset(struct aufs_super_block const *asb,
			ino_t inode_no)
{
	return sizeof(struct aufs_disk_inode) *
				(inode_no % asb->asb_inodes_in_block);
}

struct inode *aufs_inode_get(struct super_block *sb, ino_t no)
{
	struct aufs_super_block *asb = AUFS_SB(sb);
	struct buffer_head *bh;
	struct aufs_disk_inode *di;
	struct aufs_inode *ai;
	struct inode *inode;
	size_t block, offset;

	inode = iget_locked(sb, no);
	if (!inode)
		return ERR_PTR(-ENOMEM);

	if (!(inode->i_state & I_NEW))
		return inode;

	ai = AUFS_INODE(inode);
	block = aufs_inode_block(asb, no);
	offset = aufs_inode_offset(asb, no);

	pr_debug("aufs reads inode %lu from %lu block with offset %lu\n",
		(unsigned long)no, (unsigned long)block, (unsigned long)offset);

	bh = sb_bread(sb, block);
	if (!bh) {
		pr_err("cannot read block %lu\n", (unsigned long)block);
		goto read_error;
	}

	di = (struct aufs_disk_inode *)(bh->b_data + offset);
	aufs_inode_fill(ai, di);
	brelse(bh);

	inode->i_mapping->a_ops = &aufs_aops;
	if (S_ISREG(inode->i_mode)) {
		inode->i_fop = &aufs_file_ops;
	} else {
		inode->i_op = &aufs_dir_inode_ops;
		inode->i_fop = &aufs_dir_ops;
	}

	pr_debug("aufs inode %lu info:\n"
		"\tsize   = %lu\n"
		"\tblock  = %lu\n"
		"\tblocks = %lu\n"
		"\tuid    = %lu\n"
		"\tgid    = %lu\n"
		"\tmode   = %lo\n",
				(unsigned long)inode->i_ino,
				(unsigned long)inode->i_size,
				(unsigned long)ai->ai_block,
				(unsigned long)inode->i_blocks,
				(unsigned long)i_uid_read(inode),
				(unsigned long)i_gid_read(inode),
				(unsigned long)inode->i_mode);

	unlock_new_inode(inode);

	return inode;

read_error:
	pr_err("aufs cannot read inode %lu\n", (unsigned long)no);
	iget_failed(inode);

	return ERR_PTR(-EIO);
}

static int aufs_get_block(struct inode *inode, sector_t iblock,
			struct buffer_head *bh_result, int create)
{
	map_bh(bh_result, inode->i_sb, iblock + AUFS_INODE(inode)->ai_block);
	return 0;
}

static int aufs_readpage(struct file *file, struct page *page)
{
	return mpage_readpage(page, aufs_get_block);
}

static int aufs_readpages(struct file *file, struct address_space *mapping,
			struct list_head *pages, unsigned nr_pages)
{
	return mpage_readpages(mapping, pages, nr_pages, aufs_get_block);
}

static ssize_t aufs_direct_io(int rw, struct kiocb *iocb,
			struct iov_iter *iter, loff_t off)
{
	struct inode *inode = file_inode(iocb->ki_filp);

	return blockdev_direct_IO(rw, iocb, inode, iter, off, aufs_get_block);
}

const struct address_space_operations aufs_aops = {
	.readpage = aufs_readpage,
	.readpages = aufs_readpages,
	.direct_IO = aufs_direct_io
};
