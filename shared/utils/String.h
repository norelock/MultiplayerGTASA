#pragma once

#include <string>
#include <codecvt>
#include <vector>
#include <sstream>
#include <iterator>

namespace String
{
	inline constexpr char tolower(char c)
	{
		if (c >= 'A' && c <= 'Z')
			return c - 'A' + 'a';

		return c;
	}

	template<std::size_t N>
	inline constexpr unsigned int Hash(const char(&str)[N])
	{
		unsigned int value = 0, temp = 0;
		for (size_t i = 0; i < N - 1; i++)
		{
			temp = tolower(str[i]) + value;
			value = temp << 10;
			temp += value;
			value = temp >> 6;
			value ^= temp;
		}
		temp = value << 3;
		temp += value;
		unsigned int temp2 = temp >> 11;
		temp = temp2 ^ temp;
		temp2 = temp << 15;
		value = temp2 + temp;
		if (value < 2) value += 2;
		return value;
	}

	inline unsigned int Hash(const std::string& str)
	{
		unsigned int value = 0, temp = 0;
		for (size_t i = 0; i < str.length(); i++)
		{
			temp = ::tolower(str[i]) + value;
			value = temp << 10;
			temp += value;
			value = temp >> 6;
			value ^= temp;
		}
		temp = value << 3;
		temp += value;
		unsigned int temp2 = temp >> 11;
		temp = temp2 ^ temp;
		temp2 = temp << 15;
		value = temp2 + temp;
		if (value < 2) value += 2;
		return value;
	}

	inline std::string FromWString(const std::wstring& str)
	{
		static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		return conv.to_bytes(str);
	}


	inline std::string GetTimestamp() {
		auto time = std::time(nullptr);
		char buffer[30];
		std::strftime(buffer, 30, "%Y-%m-%d %H-%M-%S", std::localtime(&time));
		return std::string(buffer);
	}

	// Splits by words/spaces
	inline std::vector<std::string> Split(const std::string& str) {
		std::istringstream iss(str);
		return std::vector<std::string> {
			std::istream_iterator<std::string>{iss},
			std::istream_iterator<std::string>{}
		};
	}

	// Splits by char delimiter
	inline std::vector<std::string> Split(const std::string& str, const char delimiter) {
		std::vector<std::string> items;
		std::stringstream ss(str);
		std::string item;
		while (std::getline(ss, item, delimiter))
			items.push_back(item);

		return items;
	}

	// Splits by string delimiter
	inline std::vector<std::string> Split(const std::string& str, const std::string& delimiter) {
		std::vector<std::string> items;
		std::string line(str);

		std::size_t found = line.find(delimiter);
		while (found != std::string::npos) {
			items.push_back(line.substr(0, found));
			line.erase(0, found);
			found = line.find(delimiter);
		}

		items.push_back(line);
		return items;
	}
}

namespace WString {
	inline unsigned int Hash(const std::wstring& str)
	{
		unsigned int value = 0, temp = 0;
		for (size_t i = 0; i < str.length(); i++)
		{
			temp = tolower(str[i]) + value;
			value = temp << 10;
			temp += value;
			value = temp >> 6;
			value ^= temp;
		}
		temp = value << 3;
		temp += value;
		unsigned int temp2 = temp >> 11;
		temp = temp2 ^ temp;
		temp2 = temp << 15;
		value = temp2 + temp;
		if (value < 2) value += 2;
		return value;
	}

	inline std::wstring FromString(const std::string& str)
	{
		static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		return conv.from_bytes(str);
	}

	inline std::wstring GetTimestamp() {
		return WString::FromString(String::GetTimestamp());
	}

	// Splits by words/spaces
	inline std::vector<std::wstring> Split(const std::wstring& str) {
		std::wistringstream iss(str);
		return std::vector<std::wstring> {
			std::istream_iterator<std::wstring, wchar_t>{iss},
			std::istream_iterator<std::wstring, wchar_t>{}
		};
	}

	// Splits by char delimiter
	inline std::vector<std::wstring> Split(const std::wstring& str, const wchar_t delimiter) {
		std::vector<std::wstring> items;
		std::wstringstream ss(str);
		std::wstring item;
		while (std::getline(ss, item, delimiter))
			items.push_back(item);

		return items;
	}

	// Splits by string delimiter
	inline std::vector<std::wstring> Split(const std::wstring& str, const std::wstring& delimiter) {
		std::vector<std::wstring> items;
		std::wstring line(str);

		std::size_t found = line.find(delimiter);
		while (found != std::wstring::npos) {
			items.push_back(line.substr(0, found));
			line.erase(0, found);
			found = line.find(delimiter);
		}

		items.push_back(line);
		return items;
	}
}