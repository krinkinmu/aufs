#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "format.hpp"

size_t DeviceSize(std::string const & device)
{
	std::ifstream in(device.c_str(),
		std::ios::in | std::ios::binary | std::ios::ate);
	std::ifstream::streampos const size = in.tellg();

	return static_cast<size_t>(size);
}

bool VerifyBlocks(ConfigurationConstPtr config)
{
	if (config->Blocks() <= 3)
		return false;

	return true;
}

bool VerifyDevice(ConfigurationConstPtr config)
{
	size_t const size = DeviceSize(config->Device());

	if (size < config->Blocks() * config->BlockSize())
		return false;

	return true;
}

bool VerifyBlockSize(ConfigurationConstPtr config)
{
	if (config->BlockSize() != 512u && config->BlockSize() != 1024u &&
		config->BlockSize() != 2048u && config->BlockSize() != 4096u)
		return false;

	if (config->BlockSize() * 8 < config->Blocks())
		std::cout << "WARNING: With block size = "
			<< config->BlockSize() << " blocks number should be "
			<< "less or equal to " << config->BlockSize() * 8
			<< std::endl;

	return true;
}

ConfigurationConstPtr VerifyConfiguration(ConfigurationConstPtr config)
{
	if (!VerifyBlockSize(config))
		throw std::runtime_error("Unsupported block size");

	if (!VerifyBlocks(config))
		throw std::runtime_error("Wrong number of blocks");

	if (!VerifyDevice(config))
		throw std::runtime_error("Device is too small");

	return config;
}

void PrintHelp()
{
	std::cout << "Usage:" << std::endl
		<< "\tmkfs.aufs [(--block_size | -s) SIZE] [(--blocks | -b) BLOCKS] DEVICE"
		<< std::endl << std::endl
		<< "Where:" << std::endl
		<< "\tSIZE    - block size. Default is 4096 bytes." << std::endl
		<< "\tBLOCKS  - number of blocks would be used for aufs. By default is DEVICE size / SIZE." << std::endl
		<< "\tDEVICE  - device file." << std::endl;
}

ConfigurationConstPtr ParseArgs(int argc, char **argv)
{
	std::string device;
	size_t block_size = 4096u;
	size_t blocks = 0;

	while (argc--)
	{
		std::string const arg(*argv++);
		if ((arg == "--blocks" || arg == "-b") && argc)
		{
			blocks = std::stoi(*argv++);
			--argc;
		}
		else if ((arg == "--block_size" || arg == "-s") && argc)
		{
			block_size = std::stoi(*argv++);
			--argc;
		}
		else if (arg == "--help" || arg == "-h")
		{
			PrintHelp();
		}
		else
		{
			device = arg;
		}
	}

	if (device.empty())
		throw std::runtime_error("Device name expected");

	if (blocks == 0)
		blocks = std::min(DeviceSize(device) / block_size, block_size * 8);

	ConfigurationConstPtr config = std::make_shared<Configuration>(
		device, blocks, block_size);

	return VerifyConfiguration(config);
}

int main(int argc, char **argv)
{
	try
	{
		ConfigurationConstPtr config = ParseArgs(argc - 1, argv + 1);
		BlocksCachePtr bc = std::make_shared<BlocksCache>(config);
		SuperBlock sb(bc);
		return 0;
	}
	catch (std::exception const & e)
	{
		std::cout << "ERROR: " << e.what() << std::endl;
		PrintHelp();
	}

	return 1;
}
