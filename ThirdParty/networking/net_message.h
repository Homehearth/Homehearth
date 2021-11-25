#pragma once
#include <string>
#include <iostream>

namespace network
{
	// A message header will be sent along with a message to identify what type of message we are sending
	// using a templated class to be able to use enum class
	template <typename T>
	struct msg_header
	{
		T id = {};
		uint32_t size = 0;
	};

	// A message will include a header and a payload, the header identifies the message type.
	// The payload is the data that holds all data sent over the network. This can include position,
	// rotation or scale data and more...
	template <typename T>
	struct message
	{
		msg_header<T> header = {};
		std::vector<uint8_t> payload;

		size_t size() const
		{
			return sizeof(msg_header<T>) + payload.size();
		}

		void clear()
		{
			payload.clear();
			header = {};
		}

		friend std::ostream& operator <<(std::ostream& os, const message<T>& msg)
		{
			os << "ID:" << (int)msg.header.id << " Size: " << msg.size();

			return os;
		}

		template<typename DataType>
		friend message<T>& operator << (message<T>& msg, const DataType& data)
		{
			if constexpr (std::is_same<DataType, std::string>::value)
			{
				std::string str = std::move(data);
				// Cache the size to offset where we in memory want to copy data into
				size_t i = msg.payload.size();

				msg.payload.resize(msg.payload.size() + str.size());

				std::memcpy(msg.payload.data() + i, str.data(), str.size());

				msg << str.size();

				msg.header.size = static_cast<uint32_t>(msg.size());

				return msg;
			}
			else
			{
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to serialize");

				// Cache the size to offset where we in memory want to copy data into
				size_t i = msg.payload.size();

				msg.payload.resize(msg.payload.size() + sizeof(DataType));

				std::memcpy(msg.payload.data() + i, &data, sizeof(DataType));

				msg.header.size = static_cast<uint32_t>(msg.size());

				return msg;
			}
		}

		template<typename DataType>
		friend message<T>& operator >> (message<T>& msg, DataType& data)
		{
			if constexpr (std::is_same<DataType, std::string>::value)
			{
				std::string* str = &data;
				size_t size;
				msg >> size;

				size_t i = msg.payload.size() - size;
				str->resize(size);
				std::memcpy(str->data(), msg.payload.data() + i, size);

				msg.payload.resize(i);

				msg.header.size = static_cast<uint32_t>(msg.size());

				return msg;
			}
			else
			{
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to deserialize");

				size_t i = msg.payload.size() - sizeof(DataType);

				std::memcpy(&data, msg.payload.data() + i, sizeof(DataType));

				msg.payload.resize(i);

				msg.header.size = static_cast<uint32_t>(msg.size());

				return msg;
			}
		}
	};

	// An owned message is a message that includes a remote SOCKET to be able to know where this messages belongs to
	template <typename T>
	struct owned_message
	{
		SOCKET remote = INVALID_SOCKET;
		message<T> msg;

		friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg)
		{
			os << msg.msg;
			return os;
		}
	};
}