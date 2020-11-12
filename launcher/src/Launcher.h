#pragma once

extern std::wstring gtadrm;
extern int gtaversion;
extern bool supported;
extern std::wstring gtapath;

static std::vector<std::pair<int, std::wstring>> supportedVersions = 
{
	{ 1180, L"Other" },
	{ 1290, L"Social Club" },
	{ 1365, L"Social Club" },
	{ 1365, L"Steam" }
};

static std::vector<std::wstring> requiredFiles =
{
	L"libs\\vorbisFile.dll",
	L"libs\\lua5.1.dll",
	L"client.dll"
};