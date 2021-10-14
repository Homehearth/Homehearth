#pragma once
#include <mutex>

template<typename T>
class DoubleBuffer {
private:
	T m_data[2];
	std::atomic<bool> m_isSwapped;
public:
	DoubleBuffer();

	T& operator[](short i);

	T& GetBuffer(short i);

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
inline T& DoubleBuffer<T>::GetBuffer(short i)
{
	return &m_data[i];
}

template<typename T>
inline void DoubleBuffer<T>::Swap()
{
	std::swap(m_data[0], m_data[1]);
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

template <typename T>
class TripleBuffer
{
private:

	T m_data[3];
	std::mutex swaptex;
	std::atomic<bool> m_isSwapped;

public:

	TripleBuffer();

	T& operator[](const short& i);

	T& GetBuffer(const short& i);

	void Swap(const short& buf1, const short& buf2);

	const bool IsSwapped() const;
};

template<typename T>
inline TripleBuffer<T>::TripleBuffer()
{
}

template<typename T>
inline T& TripleBuffer<T>::operator[](const short& i)
{
	return m_data[i];
}

template<typename T>
inline T& TripleBuffer<T>::GetBuffer(const short& i)
{
	return m_data[i];
}

template<typename T>
inline void TripleBuffer<T>::Swap(const short& buf1, const short& buf2)
{
	swaptex.lock();
	if (buf1 == 0 && buf2 == 1)
		m_isSwapped = true;
	else if (buf1 == 1 && buf2 == 2)
		m_isSwapped = false;

	std::swap(m_data[buf1], m_data[buf2]);
	swaptex.unlock();
}

template<typename T>
inline const bool TripleBuffer<T>::IsSwapped() const
{
	return m_isSwapped;
}
