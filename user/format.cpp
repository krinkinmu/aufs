#include <algorithm>
#include <ctime>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bit_iterator.hpp"
#include "format.hpp"

namespace
{

uint64_t ntohll(uint64_t n)
{
	uint64_t test = 1ull;
	if (*(char *)&test == 1ull)
		return (static_cast<uint64_t>(htonl(n & 0xffffffff)) << 32u)
			| static_cast<uint64_t>(htonl(n >> 32u));
	else
		return n;
}

}

Inode::Inode(BlocksCachePtr cache, uint32_t no)
	: m_inode(no)
	, m_block(nullptr)
	, m_raw(nullptr)
{ FillInode(cache); }

uint32_t Inode::InodeNo() const noexcept
{ return m_inode; }

uint32_t Inode::FirstBlock() const noexcept
{ return ntohl(AI_FIRST_BLOCK(m_raw)); }

void Inode::SetFirstBlock(uint32_t block) noexcept
{ AI_FIRST_BLOCK(m_raw) = ntohl(block); }

uint32_t Inode::BlocksCount() const noexcept
{ return ntohl(AI_BLOCKS(m_raw)); }

void Inode::SetBlocksCount(uint32_t count) noexcept
{ AI_BLOCKS(m_raw) = ntohl(count); }

uint32_t Inode::Size() const noexcept
{ return ntohl(AI_SIZE(m_raw)); }

void Inode::SetSize(uint32_t size) noexcept
{ AI_SIZE(m_raw) = ntohl(size); }

uint32_t Inode::Gid() const noexcept
{ return ntohl(AI_GID(m_raw)); }

void Inode::SetGid(uint32_t gid) noexcept
{ AI_GID(m_raw) = ntohl(gid); }

uint32_t Inode::Uid() const noexcept
{ return ntohl(AI_UID(m_raw)); }

void Inode::SetUid(uint32_t uid) noexcept
{ AI_UID(m_raw) = ntohl(uid); }

uint32_t Inode::Mode() const noexcept
{ return ntohl(AI_MODE(m_raw)); }

void Inode::SetMode(uint32_t mode) noexcept
{ AI_MODE(m_raw) = ntohl(mode); }

uint64_t Inode::CreateTime() const noexcept
{ return ntohll(AI_CTIME(m_raw)); }


void Inode::FillInode(BlocksCachePtr cache)
{
	ConfigurationConstPtr const config = cache->Config();
	size_t const in_block = config->BlockSize() / sizeof(struct aufs_inode);
	size_t const block = InodeNo() / in_block + 3;
	size_t const index = InodeNo() % in_block;
	size_t const offset = index * sizeof(struct aufs_inode);

	m_block = cache->GetBlock(block);
	m_raw = reinterpret_cast<struct aufs_inode *>(m_block->Data() + offset);
	AI_CTIME(m_raw) = ntohll(time(NULL));
}

SuperBlock::SuperBlock(BlocksCachePtr cache)
	: m_cache(cache)
	, m_super_block(Cache()->GetBlock(0))
	, m_block_map(Cache()->GetBlock(1))
	, m_inode_map(Cache()->GetBlock(2))
{
	FillBlockMap();
	FillInodeMap();
	FillSuper();
}

BlocksCachePtr SuperBlock::Cache() noexcept
{ return m_cache; }

ConfigurationConstPtr SuperBlock::Config() const noexcept
{ return m_cache->Config(); }

InodePtr SuperBlock::AllocateInode()
{
	BitIterator const e(m_inode_map->Data() + m_inode_map->Size() * 8, 0);
	BitIterator const b(m_inode_map->Data(), 0);

	BitIterator it = std::find(b, e, true);
	if (it != e)
	{
		*it = false;
		return std::make_shared<Inode>(Cache(),
			static_cast<size_t>(it - b));
	}

	return InodePtr();
}

uint32_t SuperBlock::AllocateBlocks(size_t blocks) noexcept
{
	BitIterator const e(m_block_map->Data() + m_block_map->Size() * 8, 0);
	BitIterator const b(m_block_map->Data(), 0);

	BitIterator it = std::find(b, e, true);
	while (it != e)
	{
		BitIterator jt = std::find(it, e, false);
		if (static_cast<size_t>(jt - it) >= blocks)
		{
			std::fill(it, it + blocks, false);
			return it - b;
		}
		it = jt;
	}

	return 0;
}

void SuperBlock::FillSuper() noexcept
{
	struct aufs_super_block *sb =
		reinterpret_cast<struct aufs_super_block *>(
			m_super_block->Data());

	InodePtr root = AllocateInode();
	uint32_t block = AllocateBlocks(1);

	root->SetFirstBlock(block);
	root->SetBlocksCount(1);
	root->SetSize(0);
	root->SetUid(getuid());
	root->SetGid(getgid());
	root->SetMode(493 | S_IFDIR);

	ASB_MAGIC(sb) = htonl(AUFS_MAGIC);
	ASB_BLOCK_SIZE(sb) = htonl(Config()->BlockSize());
	ASB_ROOT_INODE(sb) = htonl(root->InodeNo());
	ASB_INODE_BLOCKS(sb) = htonl(Config()->InodeBlocks());
}

void SuperBlock::FillBlockMap() noexcept
{
	size_t const blocks = std::min(Config()->Blocks(),
		Config()->BlockSize() * 8);
	size_t const inode_blocks = Config()->InodeBlocks();

	BitIterator const it(m_block_map->Data(), 0);
	std::fill(it, it + 3 + inode_blocks, false);
	std::fill(it + 3 + inode_blocks, it + blocks, true);
	std::fill(it + blocks, it + Config()->BlockSize() * 8, false); 
}

void SuperBlock::FillInodeMap() noexcept
{
	size_t const in_block =
		Config()->BlockSize() / sizeof(struct aufs_inode);
	size_t const inode_blocks = Config()->InodeBlocks();
	size_t const inodes = std::min(inode_blocks * in_block,
		Config()->BlockSize() * 8);

	BitIterator const it(m_inode_map->Data(), 0);
	std::fill(it, it + 1, false);
	std::fill(it + 1, it + inodes, true);
	std::fill(it + inodes, it + Config()->BlockSize() * 8, false);
}
