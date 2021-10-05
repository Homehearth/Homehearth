#pragma once
#include <stdint.h>
#include <random>
#include <unordered_map>

namespace network
{
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	class UUID
	{
	private:
		uint64_t m_UUID;

	public:
		UUID(uint64_t uuid) : m_UUID(uuid)
		{

		}
		UUID() :m_UUID(s_UniformDistribution(s_Engine)) 
		{

		}
		UUID(const UUID& other) = default;

		~UUID() = default;

		operator uint64_t() const { return m_UUID; }
	};
}

namespace std
{
	template <>
	struct hash<network::UUID>
	{
		size_t operator()(const network::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}