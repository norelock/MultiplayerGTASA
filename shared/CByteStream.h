#pragma once

#include <cstdint>
#include <vector>
#include <string>

class CByteStream
{
	std::vector<uint8_t> buffer;
	uint64_t readPos = 0;

public:
	CByteStream() = default;

	CByteStream(uint64_t capacity)
	{
		buffer.reserve(capacity);
	}

	CByteStream(uint8_t* data, uint64_t size):
		buffer(data, &data[size])
	{

	}

	CByteStream(const CByteStream&) = default;
	CByteStream(CByteStream&&) = default;
	CByteStream& operator=(const CByteStream&) = default;

	const uint8_t* Data() const { return buffer.data(); }
	uint8_t* Data() { return buffer.data(); }
	std::size_t Size() const { return buffer.size(); }
	bool CanReadBytes(std::size_t bytes) { return (readPos + bytes <= buffer.size()); }
	template<class T> bool CanRead(const T&) { return CanReadBytes(sizeof(T)); }
	std::size_t BytesLeft() { return buffer.size() - readPos; }
	uint8_t* NextByte()
	{
		if (readPos < buffer.size())
			return &buffer[readPos];

		return nullptr;
	}

	void Clear()
	{
		readPos = 0;
		buffer.clear();
	}

	void Write(const uint8_t* data, std::size_t size)
	{
		if (!data || size == 0)
			return;

		buffer.insert(buffer.end(), data, &data[size]);
	}

	template<class T>
	CByteStream& operator<<(const T& val)
	{
		Write((uint8_t*)&val, sizeof(T));
		return *this;
	}

	void Read(uint8_t* data, std::size_t size)
	{
		if (!data || size == 0)
			return;

		if (readPos + size > buffer.size())
			throw std::runtime_error("reading from empty buffer");

		memcpy(data, &buffer[readPos], size);
		readPos += size;
	}

	template <typename T>
	CByteStream& operator>>(T& val)
	{
		Read((uint8_t*)&val, sizeof(T));
		return *this;
	}

	template<class T>
	friend CByteStream& operator<<(CByteStream& stream, const std::basic_string<T>& val)
	{
		uint16_t len = val.size();

		stream << len;
		stream.Write((uint8_t*)val.data(), len * sizeof(T));

		return stream;
	}

	template <class T>
	friend CByteStream& operator<<(CByteStream& stream, const std::vector<T>& val)
	{
		uint16_t size = val.size();

		stream << size;
		for (auto& el : val)
			stream << el;

		return stream;
	}

	friend CByteStream& operator<<(CByteStream& stream, const CByteStream& val)
	{
		uint64_t size = val.Size();
		stream << size;
		stream.Write(val.Data(), size);

		return stream;
	}

	friend CByteStream& operator<<(CByteStream& stream, const MValue& val)
	{
		MValue::Type type = val.GetType();
		stream << type;

		switch (type)
		{
		case MValue::BOOL:
			stream << val.Get<MValue::BOOL>();
			break;
		case MValue::INT:
			stream << val.Get<MValue::INT>();
			break;
		case MValue::UINT:
			stream << val.Get<MValue::UINT>();
			break;
		case MValue::DOUBLE:
			stream << val.Get<MValue::DOUBLE>();
			break;
		case MValue::STRING:
			stream << val.Get<MValue::STRING>();
			break;
		case MValue::ARRAY:
		{
			const MValue::Array& arr = val.Get<MValue::ARRAY>();

			uint16_t size = arr.size();
			stream << size;

			for (uint16_t i = 0; i < size; ++i)
				stream << arr[i];

			break;
		}
		case MValue::DICT:
		{
			const MValue::Dict& dict = val.Get<MValue::DICT>();

			uint16_t size = dict.size();
			stream << size;

			for (auto& el : dict)
			{
				uint8_t keytype = el.first.index();

				stream << keytype;

				if (keytype == 0)
					stream << std::get<0>(el.first);
				else
					stream << std::get<1>(el.first);

				stream << el.second;
			}

			break;
		}
		case MValue::ENTITY:
			stream << val.Get<MValue::ENTITY>().id;
			break;
		}

		return stream;
	}

	template<class T>
	friend CByteStream& operator>>(CByteStream& stream, std::basic_string<T>& val)
	{
		uint16_t len;
		stream >> len;

		if (len > 0)
		{
			val.resize(len);
			stream.Read((uint8_t*)val.data(), len * sizeof(T));
		}

		return stream;
	}

	template <class T>
	friend CByteStream& operator>>(CByteStream& stream, std::vector<T>& val)
	{
		uint16_t size;
		stream >> size;
		val.resize(size);

		for (int i = 0; i < size; ++i)
			stream >> val[i];

		return stream;
	}

	friend CByteStream& operator>>(CByteStream& stream, CByteStream& val)
	{
		uint64_t size;
		stream >> size;

		if (size > 0)
		{
			val.buffer.resize(size);
			stream.Read(val.Data(), size);
		}

		return stream;
	}

	friend CByteStream& operator>>(CByteStream& stream, MValue& val)
	{
		MValue::Type type;
		stream >> type;

		switch (type)
		{
		case MValue::BOOL:
		{
			bool _val;
			stream >> _val;
			val = _val;
			break;
		}
		case MValue::INT:
		{
			int64_t _val;
			stream >> _val;
			val = _val;
			break;
		}
		case MValue::UINT:
		{
			uint64_t _val;
			stream >> _val;
			val = _val;
			break;
		}
		case MValue::DOUBLE:
		{
			double _val;
			stream >> _val;
			val = _val;
			break;
		}
		case MValue::STRING:
		{
			std::string _val;
			stream >> _val;
			val = _val;
			break;
		}
		case MValue::ARRAY:
		{
			uint16_t size;
			stream >> size;

			val = MValue::CreateArray(size);

			for (uint16_t i = 0; i < size; ++i)
				stream >> val[i];

			break;
		}
		case MValue::DICT:
		{
			val = MValue::CreateDict();

			uint16_t size;
			stream >> size;
			
			for (int i = 0; i < size; ++i)
			{
				MValue::KeyType key;
				uint8_t keytype;
				stream >> keytype;

				if (keytype == 0)
				{
					std::string _key;
					stream >> _key;
					key = _key;
				}
				else
				{
					int _key;
					stream >> _key;
					key = _key;
				}

				stream >> val[key];
			}

			break;
		}
		case MValue::ENTITY:
		{
			uint16_t id;
			stream >> id;

			val = MValue::CreateEntity(id);

			break;
		}
		default:
			val = MValue::CreateNil();
		}

		return stream;
	}
};
