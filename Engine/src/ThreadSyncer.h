#include "DoubleBuffer.h"
#pragma once

namespace thread
{
	class ThreadSyncer
	{
	private:

		static ThreadSyncer* instance;
		ThreadSyncer();
		~ThreadSyncer();

	public:

		// Used for safe exit during program closure.
		static bool s_engineRunning;
		static bool s_drawRunning;
		static bool s_networkRunning;

	public:

		static void Initialize();
		static void Destroy();

	};

	/*
		Handles the triple buffering for the draw and update thread.
	*/

	/*
	template<class T>
	class TripleBuffer
	{
	private:
		
		T** m_buffers;
		std::mutex m_mutex;

		bool m_isAllocated = false;
	public:

		TripleBuffer();
		~TripleBuffer();

		/*
			Allocate up spots for the buffers.
		
		const bool AllocateBuffers();
		/*
			Copies the parameter buffer into indexed buffer spot.
			If data was previously allocated to this buffer it will be deleted.
		
		const bool SetUpBuffer(const int&& index, T&& p_buffer);
		/*
			Swap the buffer pointers between _first and _second
		
		void SwapBuffers(const int&& _first, const int&& _second);

		/*
			Return a pointer to the buffer at indexed spot.
		
		const bool GetBuffer(const int&& index, T ** p_pointer);

		/*
			Get the pointer to a buffer.
		
		T* GetBufferUnSafe(const int&& index);

		/*
			Set the buffer at position index to the preallocated pointer address.
		
		const bool SetPreAllocatedBuffer(const int&& index, T* p_pointer);
	};

	/*
		Triplebuffer class functions
	
	template<class T>
	inline TripleBuffer<T>::TripleBuffer()
	{
		m_buffers = new T * [3];
		for (int i = 0; i < 3; i++)
			m_buffers[i] = nullptr;
	}
	template<class T>
	inline TripleBuffer<T>::~TripleBuffer()
	{
		if (m_isAllocated)
		{
			if (m_buffers[0])
				delete m_buffers[0];
			if (m_buffers[1])
				delete m_buffers[1];
			if (m_buffers[2])
				delete m_buffers[2];
		}

		delete[] m_buffers;
	}
	template<class T>
	inline const bool TripleBuffer<T>::AllocateBuffers()
	{
		for (int i = 0; i < 3; i++)
		{
			T * temp = new T();
			m_buffers[i] = temp;
		}
		m_isAllocated = true;
		return true;
	}
	template<class T>
	inline const bool TripleBuffer<T>::SetUpBuffer(const int&& index, T&& p_buffer)
	{

		if (index >= 3 || index < 0)
			[] {LOG_WARNING("Index was out of range from 0 to 3."); return false; };
		m_buffers[index] = &p_buffer;
		return true;
	}
	template<class T>
	inline void TripleBuffer<T>::SwapBuffers(const int&& _first, const int&& _second)
	{
		std::swap(m_buffers[_first], m_buffers[_second]);
	}
	template<class T>
	inline const bool TripleBuffer<T>::GetBuffer(const int&& index, T** p_pointer)
	{
		if (index < 3 || index >= 0)
		{
			*p_pointer = m_buffers[index];
			return true;
		}
		else
		{
			return false;
		}
	}

	template<class T>
	inline T* TripleBuffer<T>::GetBufferUnSafe(const int&& index)
	{
		if (index < 2 || index >= 0)
			return m_buffers[index];
		else
			return nullptr;
	}

	template<class T>
	inline const bool TripleBuffer<T>::SetPreAllocatedBuffer(const int&& index, T* p_pointer)
	{
		if (index > 2 || index < 0)
			return false;

		m_buffers[index] = p_pointer;

		return true;
	}

	*/

	/*
	template <class T>
	class DoubleBuffer
	{
	private:

		T** m_buffers;

		bool m_isAllocated = false;
		bool m_isSwapped = false;
	public:

		DoubleBuffer();
		~DoubleBuffer();

		const bool AllocateBuffers();

		T* GetBuffer(const int&& index);

		const bool SwapBuffers();

		const bool IsSwapped();

		void ReadySwap();
	};

	template<class T>
	inline DoubleBuffer<T>::DoubleBuffer()
	{
		m_buffers = new T * [2];
	}

	template<class T>
	inline DoubleBuffer<T>::~DoubleBuffer()
	{
		if (m_isAllocated)
		{
			if (m_buffers[0])
				delete m_buffers[0];
			if (m_buffers[1])
				delete m_buffers[1];
		}

		delete[] m_buffers;
	}

	template<class T>
	inline const bool DoubleBuffer<T>::AllocateBuffers()
	{
		for (int i = 0; i < 2; i++)
		{
			T* temp = new T();
			m_buffers[i] = temp;
		}

		m_isAllocated = true;
		return true;
	}

	template<class T>
	inline T* DoubleBuffer<T>::GetBuffer(const int&& index)
	{
		if (index == 0 || index == 1)
			return m_buffers[index];
		else
			return nullptr;
	}
	template<class T>
	inline const bool DoubleBuffer<T>::SwapBuffers()
	{
		std::swap(m_buffers[0], m_buffers[1]);
		m_isSwapped = true;
		return true;
	}
	template<class T>
	inline const bool DoubleBuffer<T>::IsSwapped()
	{
		return m_isSwapped;
	}
	template<class T>
	inline void DoubleBuffer<T>::ReadySwap()
	{
		m_isSwapped = false;
	}
	*/
}