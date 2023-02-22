#pragma once
#include "SkyCommons/SkyCommons.h"
#include <fstream>
#include <memory>

static bool LoadFile(const char* path, uint32& size, std::shared_ptr<char[]>& buf)
{
	std::ifstream file(path, std::ios::binary | std::ios::beg);
	if (!file.is_open())
		return false;

	std::streampos fsize = file.tellg();
	file.seekg(0, std::ios::end);
	fsize = file.tellg() - fsize;
	file.seekg(0, std::ios::beg);
	size = fsize;
	buf = std::shared_ptr<char[]>(new char[size]);
	file.read(buf.get(), size);
	file.close();
	return true;
}
