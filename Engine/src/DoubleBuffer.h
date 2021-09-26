#pragma once
#include <mutex>

template<typename T>
class DoubleBuffer {
private:
	T m_data[2];
	std::atomic<bool> m_isSwapped;
	std::mutex m_mutex;
public:
	DoubleBuffer();

	T& operator[](short i);

	void Swap();

	void ReadyForSwap();

	bool IsSwapped() const;
};

template<typename T>
inline DoubleBuffer<T>::DoubleBuffer() 
	: m_isSwapped(false)
{
}

template<typename T>
inline T& DoubleBuffer<T>::operator[](short i)
{
	return m_data[i];	
}

template<typename T>
inline void DoubleBuffer<T>::Swap()
{
	m_mutex.lock();
	std::swap(m_data[0], m_data[1]);
	m_mutex.unlock();
	m_isSwapped = true;
}

template<typename T>
inline void DoubleBuffer<T>::ReadyForSwap()
{
	m_isSwapped = false;
}

template<typename T>
inline bool DoubleBuffer<T>::IsSwapped() const
{
	return m_isSwapped;
}
