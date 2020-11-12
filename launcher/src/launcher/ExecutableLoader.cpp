#include "stdafx.h"
/*
#pragma bss_seg(".cdummy")
char dummy_seg[0x2500000];

#pragma data_seg(".zdata")
char zdata[0x50000] = { 1 };
*/
ExecutableLoader::ExecutableLoader(const BYTE* origBinary)
{
	m_origBinary = origBinary;
	m_loadLimit = UINT_MAX;

	SetLibraryLoader([](const char* name)
	{
		return LoadLibraryA(name);
	});

	SetFunctionResolver([](HMODULE module, const char* name)
	{
		return reinterpret_cast<LPVOID>(GetProcAddress(module, name));
	});
}

std::wstring s2ws(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

bool ExecutableLoader::LoadDependentLibraries(IMAGE_NT_HEADERS* ntHeader)
{
	IMAGE_DATA_DIRECTORY* importDirectory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	IMAGE_IMPORT_DESCRIPTOR* descriptor = GetTargetRVA<IMAGE_IMPORT_DESCRIPTOR>(importDirectory->VirtualAddress);

	while (descriptor->Name)
	{
		const char* name = GetTargetRVA<char>(descriptor->Name);

		HMODULE module = ResolveLibrary(name);

		if (!module)
		{
			std::ostringstream os;
			os << GetLastError();
			std::string str = "Could not load dependent module " + std::string(name) + ". Error code: " + os.str();

			MessageBox(NULL, s2ws(str).c_str(), L"Launch error", MB_OK);
			//FatalError(va("Could not load dependent module %s. Error code was %i.", name, GetLastError()));
			return false;
		}

		if (reinterpret_cast<unsigned>(module) == 0xFFFFFFFF)
		{
			descriptor++;
			continue;
		}

		auto nameTableEntry = GetTargetRVA<unsigned>(descriptor->OriginalFirstThunk);
		auto addressTableEntry = GetTargetRVA<unsigned>(descriptor->FirstThunk);

		while (*nameTableEntry)
		{
			FARPROC function;
			const char* functionName;

			if (IMAGE_SNAP_BY_ORDINAL(*nameTableEntry))
			{
				function = reinterpret_cast<FARPROC>(
					ResolveLibraryFunction(module, MAKEINTRESOURCEA(IMAGE_ORDINAL(*nameTableEntry))));
				char buff[256];
				snprintf(buff, sizeof(buff), "#%d", IMAGE_ORDINAL(*nameTableEntry));
				std::string buffAsStdStr = buff;
				functionName = buffAsStdStr.c_str();
			}
			else
			{
				auto import = GetTargetRVA<IMAGE_IMPORT_BY_NAME>(*nameTableEntry);

				function = reinterpret_cast<FARPROC>(ResolveLibraryFunction(module, (const char*)import->Name));
				functionName = static_cast<const char*>(import->Name);
			}

			if (!function)
			{
				char pathName[MAX_PATH];
				GetModuleFileNameA(module, pathName, sizeof(pathName));

				MessageBox(NULL, L"Could not load function in dependent module", L"Launch error", MB_OK);
				//FatalError(va("Could not load function %s in dependent module %s (%s).", functionName, name, pathName));
				return false;
			}

			*addressTableEntry = reinterpret_cast<unsigned>(function);

			nameTableEntry++;
			addressTableEntry++;
		}

		descriptor++;
	}

	return true;
}

void ExecutableLoader::LoadSection(IMAGE_SECTION_HEADER* section)
{
	void* targetAddress = GetTargetRVA<uint8_t>(section->VirtualAddress);
	const void* sourceAddress = m_origBinary + section->PointerToRawData;

	if ((uintptr_t)targetAddress >= m_loadLimit)
	{
		MessageBox(NULL, L"Exceeded load limit.", L"Launch error", MB_OK);
		//FatalError("Exceeded load limit.");
		return;
	}

	if (section->SizeOfRawData > 0)
	{
		uint32_t sizeOfData = section->SizeOfRawData < section->Misc.VirtualSize ? section->SizeOfRawData : section->Misc.VirtualSize;

		DWORD oldProtect;
		VirtualProtect(targetAddress, sizeOfData, PAGE_EXECUTE_READWRITE, &oldProtect);

		memcpy(targetAddress, sourceAddress, sizeOfData);
	}
}

void ExecutableLoader::LoadSections(IMAGE_NT_HEADERS* ntHeader)
{
	IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);

	for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
	{
		// Load the given section into memory
		LoadSection(section);

		section++;
	}
}

void ExecutableLoader::LoadIntoModule(HMODULE module)
{
	m_executableHandle = module;

	IMAGE_DOS_HEADER* header = (IMAGE_DOS_HEADER*)m_origBinary;

	if (header->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(NULL, L"Cannot find dos signature.", L"Launch error", MB_OK);
		return;
	}

	IMAGE_DOS_HEADER* sourceHeader = (IMAGE_DOS_HEADER*)module;
	IMAGE_NT_HEADERS* sourceNtHeader = GetTargetRVA<IMAGE_NT_HEADERS>(sourceHeader->e_lfanew);

	IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(m_origBinary + header->e_lfanew);

	LoadSections(ntHeader);
	LoadDependentLibraries(ntHeader);

	m_entryPoint = GetTargetRVA<void>(ntHeader->OptionalHeader.AddressOfEntryPoint);

	DWORD oldProtect;
	VirtualProtect(sourceNtHeader, 0x1000, PAGE_EXECUTE_READWRITE, &oldProtect);

	sourceNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT] = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
}

HMODULE ExecutableLoader::ResolveLibrary(const char* name)
{
	return m_libraryLoader(name);
}

LPVOID ExecutableLoader::ResolveLibraryFunction(HMODULE module, const char* name)
{
	return m_functionResolver(module, name);
}