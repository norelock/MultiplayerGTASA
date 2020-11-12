#pragma once

#include "microtar.h"
#include <string>
#include <fstream>
#include <vector>

class CPackage
{
public:
	enum Mode {
		Read = 1,
		Write = 2,
		ReadWrite = 3,
	};

private:
	Mode mode;
	mtar_t tarball;
	bool edited;

public:
	CPackage() :edited(false) { }

	bool Open(const std::string& path, Mode _mode)
	{
		mode = _mode;
		std::string smode;

		if (mode & Read)
			smode.append("r");

		if (mode & Write)
			smode.append("w");

		return mtar_open(&tarball, path.c_str(), smode.c_str()) == 0;
	}

	bool WriteFile(const std::string& dest, const std::string& path)
	{
		std::ifstream ifile(path, std::ios::binary);
		std::vector<char> data(
			(std::istreambuf_iterator<char>(ifile)),
			(std::istreambuf_iterator<char>())
		);

		edited = true;
		mtar_write_file_header(&tarball, dest.c_str(), data.size());
		return mtar_write_data(&tarball, data.data(), data.size()) == 0;
	}

	bool WriteString(const std::string& dest, const std::string& str)
	{
		edited = true;
		mtar_write_file_header(&tarball, dest.c_str(), str.size());
		return mtar_write_data(&tarball, str.c_str(), str.size()) == 0;
	}

	bool FileExists(const std::string& path)
	{
		mtar_header_t h;
		int err = mtar_find(&tarball, path.c_str(), &h);
		return err == MTAR_ESUCCESS;
	}

	std::vector<char> ReadFile(const std::string& path)
	{
		mtar_header_t h;
		std::vector<char> data;

		int err = mtar_find(&tarball, path.c_str(), &h);
		if (err != MTAR_ESUCCESS)
			return data;

		data.resize(h.size);
		mtar_read_data(&tarball, data.data(), h.size);

		return data;
	}

	~CPackage()
	{
		if (edited)
			mtar_finalize(&tarball);
		mtar_close(&tarball);
	}
};