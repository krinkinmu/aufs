#ifndef __FORMAT_HPP__
#define __FORMAT_HPP__

#include "block.hpp"

class Inode
{
public:
	explicit Inode(BlocksCache &cache, uint32_t no);

	uint32_t InodeNo() const noexcept;
	uint32_t FirstBlock() const noexcept;
	void SetFirstBlock(uint32_t block) noexcept;
	uint32_t BlocksCount() const noexcept;
	void SetBlocksCount(uint32_t count) noexcept;
	uint32_t Size() const noexcept;
	void SetSize(uint32_t size) noexcept;
	uint32_t Gid() const noexcept;
	void SetGid(uint32_t gid) noexcept;
	uint32_t Uid() const noexcept;
	void SetUid(uint32_t uid) noexcept;
	uint32_t Mode() const noexcept;
	void SetMode(uint32_t mode) noexcept;
	uint64_t CreateTime() const noexcept;

private:
	void FillInode(BlocksCache &cache);

	uint32_t		m_inode;
	BlockPtr		m_block;
	struct aufs_inode *	m_raw;
};

using InodePtr = std::shared_ptr<Inode>;
using InodeConstPtr = std::shared_ptr<Inode const>;


class SuperBlock
{
public:
	explicit SuperBlock(BlocksCache &cache);

	ConfigurationConstPtr Config() const noexcept;
	uint32_t AllocateInode() noexcept;
	uint32_t AllocateBlocks(size_t blocks) noexcept;
	void SetRootInode(uint32_t root) noexcept;

private:
	void FillSuper(BlocksCache &cache) noexcept;
	void FillBlockMap(BlocksCache &cache) noexcept;
	void FillInodeMap(BlocksCache &Cache) noexcept;

	BlockPtr	m_super_block;
	BlockPtr	m_block_map;
	BlockPtr	m_inode_map;
};

class Formatter
{
public:
	Formatter(ConfigurationConstPtr config)
		: m_config(config)
		, m_cache(config)
		, m_super(m_cache)
	{ }

	void SetRootInode(InodePtr const &inode) noexcept;
	InodePtr MkDir(uint32_t entries);
	InodePtr MkFile(uint32_t size);

	uint32_t Write(InodePtr &inode, uint8_t const *data,
			uint32_t size) noexcept;

	void AddChild(InodePtr &inode, char const *name,
			InodePtr const &ch) noexcept;

private:
	ConfigurationConstPtr	m_config;
	BlocksCache		m_cache;
	SuperBlock		m_super;
};

#endif /*__FORMAT_HPP__*/
