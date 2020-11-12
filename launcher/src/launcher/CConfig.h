#pragma once

class CConfig : public CSingleton<CConfig>
{
	bool isDebug;
	std::string name;
	std::string lang;
	std::wstring gamePath;
	std::string lastIp;

public:
	CConfig() :
		isDebug(false),
		name("altSAPlayer"),
		lang("en")
	{

	}

	bool Load(const std::wstring& fileName)
	{
		using json = nlohmann::json;
		json config;

		try
		{
			std::ifstream ifile(fileName);
			ifile >> config;
		}
		catch (json::parse_error)
		{
			return false;
		}

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

		try
		{
			name = config["name"].get<std::string>();
			isDebug = config["debug"].get<bool>();
			lang = config["lang"].get<std::string>();
			gamePath = converter.from_bytes(config["gtapath"].get<std::string>());
			lastIp = config["lastip"].get<std::string>();
		}
		catch (json::parse_error)
		{
			return false;
		}

		return true;
	}

	bool Save(const std::wstring& filename)
	{
		using json = nlohmann::json;
		json config;

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

		config["name"] = name;
		config["debug"] = isDebug;
		config["lang"] = lang;
		config["gtapath"] = converter.to_bytes(gamePath);
		config["lastip"] = lastIp;


		std::ofstream ofile(filename);
		ofile << std::setw(4) << config << std::endl;

		return true;
	}

	inline const std::wstring& GetPath() { return gamePath; }
	inline void SetPath(const std::wstring& path) { gamePath = path; }
};
