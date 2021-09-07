#pragma once

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

		friend std::ostream& operator <<(std::ostream& os, const message<T>& msg)
		{
			os << "ID:" << (int)msg.header.id << " Size: " << msg.size();

			return os;
		}

		template<typename DataType>
		friend message<T>& operator << (message<T>& msg, const DataType& data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to serialize");

			// Cache the size to offset where we in memory want to copy data into
			size_t i = msg.payload.size();

			msg.payload.resize(msg.payload.size() + sizeof(DataType));

			std::memcpy(msg.payload.data() + i, &data, sizeof(DataType));

			msg.header.size = msg.size();

			return msg;
		}

		template<typename DataType>
		friend message<T>& operator >> (message<T>& msg, DataType& data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to deserialize");

			size_t i = msg.payload.size() - sizeof(DataType);

			std::memcpy(&data, msg.payload.data() + i, sizeof(DataType));

			msg.payload.resize(i);

			msg.header.size = msg.size();

			return msg;
		}
	};
}