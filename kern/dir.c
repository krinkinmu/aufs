#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/pagemap.h>

#include "aufs.h"

static size_t aufs_dir_offset(size_t idx)
{
	return idx * sizeof(struct aufs_disk_dir_entry);
}

static size_t aufs_dir_pages(struct inode *inode)
{
	size_t size = aufs_dir_offset(inode->i_size);

	return (size + PAGE_CACHE_SIZE - 1) >> PAGE_CACHE_SHIFT;
}

static size_t aufs_dir_entry_page(size_t idx)
{
	return aufs_dir_offset(idx) >> PAGE_CACHE_SHIFT;
}

static inline size_t aufs_dir_entry_offset(size_t idx)
{
	return aufs_dir_offset(idx) -
			(aufs_dir_entry_page(idx) << PAGE_CACHE_SHIFT);
}

static struct page *aufs_get_page(struct inode *inode, size_t n)
{
	struct address_space *mapping = inode->i_mapping;
	struct page *page = read_mapping_page(mapping, n, NULL);

	if (!IS_ERR(page))
		kmap(page);
	return page;
}

static void aufs_put_page(struct page *page)
{
	kunmap(page);
	put_page(page);
}

static int aufs_dir_emit(struct dir_context *ctx,
			struct aufs_disk_dir_entry *de)
{
	unsigned type = DT_UNKNOWN;
	unsigned len = strlen(de->dde_name);
	size_t ino = be32_to_cpu(de->dde_inode);

	return dir_emit(ctx, de->dde_name, len, ino, type);
}

static int aufs_iterate(struct inode *inode, struct dir_context *ctx)
{
	size_t pages = aufs_dir_pages(inode);
	size_t pidx = aufs_dir_entry_page(ctx->pos);
	size_t off = aufs_dir_entry_offset(ctx->pos);

	for ( ; pidx < pages; ++pidx, off = 0) {
		struct page *page = aufs_get_page(inode, pidx);
		struct aufs_disk_dir_entry *de;
		char *kaddr;

		if (IS_ERR(page)) {
			pr_err("cannot access page %u in %lu", pidx,
						(unsigned long)inode->i_ino);
			return PTR_ERR(page);
		}

		kaddr = page_address(page);
		de = (struct aufs_disk_dir_entry *)(kaddr + off);
		while (off < PAGE_CACHE_SIZE && ctx->pos < inode->i_size) {
			if (!aufs_dir_emit(ctx, de)) {
				aufs_put_page(page);
				return 0;
			}
			++ctx->pos;
			++de;
		}
		aufs_put_page(page);
	}
	return 0;
}

static int aufs_readdir(struct file *file, struct dir_context *ctx)
{
	return aufs_iterate(file_inode(file), ctx);
}

const struct file_operations aufs_dir_ops = {
	.llseek = generic_file_llseek,
	.read = generic_read_dir,
	.iterate = aufs_readdir,
};

struct aufs_filename_match {
	struct dir_context ctx;
	ino_t ino;
	const char *name;
	int len;
};

static int aufs_match(struct dir_context *ctx, const char *name, int len,
			loff_t off, u64 ino, unsigned type)
{
	struct aufs_filename_match *match = (struct aufs_filename_match *)ctx;

	if (len != match->len)
		return 0;

	if (memcmp(match->name, name, len) == 0) {
		match->ino = ino;
		return 1;
	}
	return 0;
}

static ino_t aufs_inode_by_name(struct inode *dir, struct qstr *child)
{
	struct aufs_filename_match match = {
		{ &aufs_match, 0 }, 0, child->name, child->len
	};

	int err = aufs_iterate(dir, &match.ctx);

	if (err)
		pr_err("Cannot find dir entry, error = %d", err);
	return match.ino;
}

static struct dentry *aufs_lookup(struct inode *dir, struct dentry *dentry,
			unsigned flags)
{
	struct inode *inode = NULL;
	ino_t ino;

	if (dentry->d_name.len >= AUFS_DDE_MAX_NAME_LEN)
		return ERR_PTR(-ENAMETOOLONG);

	ino = aufs_inode_by_name(dir, &dentry->d_name);
	if (ino) {
		inode = aufs_inode_get(dir->i_sb, ino);
		if (IS_ERR(inode)) {
			pr_err("Cannot read inode %lu", (unsigned long)ino);
			return ERR_PTR(PTR_ERR(inode));
		}
		d_add(dentry, inode);
	}
	return NULL;
}

const struct inode_operations aufs_dir_inode_ops = {
	.lookup = aufs_lookup,
};
