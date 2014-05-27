#ifndef __BLOCK_HPP__
#define __BLOCK_HPP__

#include <cstddef>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

class Configuration
{
public:
	explicit Configuration(std::string device,
			size_t blocks,
			size_t block_size) noexcept
		: m_device(device)
		, m_device_blocks(blocks)
		, m_block_size(block_size)
	{ }

	std::string const & Device() const noexcept
	{ return m_device; }

	size_t Blocks() const noexcept
	{ return m_device_blocks; }

	size_t BlockSize() const noexcept
	{ return m_block_size; }

private:
	std::string	m_device;
	size_t		m_device_blocks;
	size_t		m_block_size;
};

using ConfigurationPtr = std::shared_ptr<Configuration>;
using ConfigurationConstPtr = std::shared_ptr<Configuration const>;


class Block
{
public:
	explicit Block(ConfigurationConstPtr config, size_t no)
		: m_config(config)
		, m_number(no)
		, m_data(m_config->BlockSize(), 0)
	{ }

	Block(Block const &) = delete;
	Block & operator=(Block const &) = delete;

	Block(Block &&) = delete;
	Block & operator=(Block &&) = delete;

	size_t BlockNo() const noexcept
	{ return m_number; }

	uint8_t * Data() noexcept
	{ return m_data.data(); }

	uint8_t const * Data() const noexcept
	{ return m_data.data(); }

	size_t Size() const noexcept
	{ return m_config->BlockSize(); }

	std::ostream & Dump(std::ostream & out) const
	{ return out.write(reinterpret_cast<char const *>(Data()), Size()); }

	std::istream & Read(std::istream & in)
	{ return in.read(reinterpret_cast<char *>(Data()), Size()); }

private:
	ConfigurationConstPtr	m_config;
	size_t			m_number;
	std::vector<uint8_t>	m_data;
};

using BlockPtr = std::shared_ptr<Block>;
using BlockConstPtr = std::shared_ptr<Block const>;

#endif /*__BLOCK_HPP__*/
