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

		// Used for sync between threads to prevent read/write on same block.
		static int s_engineIndex;
		static int s_drawIndex;
		static int s_networkIndex;

	public:

		static void Initialize();
		static void Destroy();

	};

	/*
		Handles the triple buffering for the draw and update thread.
	*/
	template<class T>
	class TripleBuffer
	{
	private:
		
		T** m_buffers;

		// Atomics to prevent read/write?
		/*
		std::atomic<T> m_buffer1;
		std::atomic<T> m_buffer2;
		std::atomic<T> m_buffer3;
		*/
		std::mutex m_mutex;

	public:

		TripleBuffer();
		~TripleBuffer();
		/*
			Copies the parameter buffer into indexed buffer spot.
			If data was previously allocated to this buffer it will be deleted.
		*/
		const bool SetUpBuffer(const int&& index, T&& p_buffer);
		/*
			Swap the buffer pointers between _first and _second
		*/
		void SwapBuffers(const int&& _first, const int&& _second);

		/*
			Return a pointer to the buffer at indexed spot.
		*/
		const bool GetBuffer(const int&& index, T * p_pointer);

		/*
			Set the buffer at position index to the preallocated pointer address.
		*/
		const bool SetPreAllocatedBuffer(const int&& index, T* p_pointer);
	};

	/*
		Triplebuffer class functions
	*/
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
		for (int i = 0; i < 3; i++)
		{
			//if (m_buffers[i])
				//delete m_buffers[i];
		}

		delete[] m_buffers;
	}
	template<class T>
	inline const bool TripleBuffer<T>::SetUpBuffer(const int&& index, T&& p_buffer)
	{

		if (index >= 3 || index < 0)
			[] {LOG_WARNING("Index was out of range from 0 to 3."); return false; };
		m_mutex.lock();
		m_buffers[index] = &p_buffer;
		m_mutex.unlock();
		return true;
	}
	template<class T>
	inline void TripleBuffer<T>::SwapBuffers(const int&& _first, const int&& _second)
	{
		m_mutex.lock();
		std::swap(m_buffers[_first], m_buffers[_second]);
		m_mutex.unlock();
	}
	template<class T>
	inline const bool TripleBuffer<T>::GetBuffer(const int&& index, T* p_pointer)
	{
		if (index < 3 || index >= 0)
		{
			m_mutex.lock();
			p_pointer = m_buffers[index];
			m_mutex.unlock();
			return true;
		}
		else
		{
			return false;
		}
	}

	template<class T>
	inline const bool TripleBuffer<T>::SetPreAllocatedBuffer(const int&& index, T* p_pointer)
	{
		if (index > 2 || index < 0)
			return false;

		m_buffers[index] = p_pointer;

		return true;
	}

	class Buff
	{
	public:
		int** buffer = nullptr;
		void Setup(int&& size)
		{
			this->buffer = new int * [size];
		}
		~Buff()
		{
			if (this->buffer)
				delete[] this->buffer;
		}
	};
}

// Engine Index
#define eDex thread::ThreadSyncer::instance->s_engineIndex

// Draw Index
#define dDex thread::ThreadSyncer::instance->s_drawIndex

// Network Index
#define nDex thread::ThreadSyncer::instance->s_networkIndex