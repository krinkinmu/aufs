#ifndef __FORMAT_HPP__
#define __FORMAT_HPP__

#include "block.hpp"

class Inode
{
public:
	explicit Inode(BlocksCachePtr cache, uint32_t no);

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
	void FillInode(BlocksCachePtr cache);

	uint32_t		m_inode;
	BlockPtr		m_block;
	struct aufs_inode *	m_raw;
};

using InodePtr = std::shared_ptr<Inode>;
using InodeConstPtr = std::shared_ptr<Inode const>;


class SuperBlock
{
public:
	explicit SuperBlock(BlocksCachePtr cache);

	BlocksCachePtr Cache() noexcept;
	ConfigurationConstPtr Config() const noexcept;
	InodePtr AllocateInode();
	uint32_t AllocateBlocks(size_t blocks) noexcept;

private:
	void FillSuper() noexcept;
	void FillBlockMap() noexcept;
	void FillInodeMap() noexcept;

	BlocksCachePtr	m_cache;
	BlockPtr	m_super_block;
	BlockPtr	m_block_map;
	BlockPtr	m_inode_map;
};

#endif /*__FORMAT_HPP__*/
