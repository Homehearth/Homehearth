#pragma once

#include "net_common.h"
#include "net_message.h"

namespace network
{
	template <typename T>
	class tsQueue
	{
	private:
		std::mutex mutex;
		std::deque<T> deqQueue;

	public:
		tsQueue() = default;
		tsQueue(const tsQueue<T>& other) = delete;
		virtual ~tsQueue() { this->clear(); }

	public:
		const T& front()
		{
			std::scoped_lock lock(mutex);

			return deqQueue.front();
		}

		const T& back()
		{
			std::scoped_lock lock(mutex);

			return deqQueue.back();
		}

		void push_back(const T& item)
		{
			std::scoped_lock lock(mutex);

			deqQueue.emplace_back(item);
		}

		void push_front(const T& item)
		{
			std::scoped_lock lock(mutex);

			deqQueue.emplace_front(item);
		}

		size_t count()
		{
			std::scoped_lock lock(mutex);

			return deqQueue.size();
		}

		bool empty()
		{
			std::scoped_lock lock(mutex);

			return deqQueue.empty();
		}

		void clear()
		{
			std::scoped_lock lock(mutex);

			deqQueue.clear();
		}

		T pop_front()
		{
			std::scoped_lock lock(mutex);
			auto t = std::move(deqQueue.front());
			deqQueue.pop_front();
			return t;
		}
		
		T pop_back()
		{
			std::scoped_lock lock(mutex);
			auto t = std::move(deqQueue.back());
			deqQueue.pop_back();
			return t;
		}
	};
}