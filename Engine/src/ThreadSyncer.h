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
		
		T* m_buffers[3];

	public:

		TripleBuffer();
		~TripleBuffer();
		/*
			Copies the parameter buffer into indexed buffer spot.
			If data was previously allocated to this buffer it will be deleted.
		*/
		const bool SetUpBuffer(const int&& index, const T& p_buffer);
		/*
			Swap the buffer pointers between _first and _second
		*/
		void SwapBuffers(const int&& _first, const int&& _second);

		/*
			Return a pointer to the buffer at indexed spot.
		*/
		T* GetBuffer(const int&& index);
	};

	/*
		Triplebuffer class functions
	*/
	template<class T>
	inline TripleBuffer<T>::TripleBuffer()
	{
		for (int i = 0; i < 3; i++)
			m_buffers[i] = nullptr;
	}
	template<class T>
	inline TripleBuffer<T>::~TripleBuffer()
	{
		for (int i = 0; i < 3; i++)
		{
			if (m_buffers[i])
				delete m_buffers[i];
		}
	}
	template<class T>
	inline const bool TripleBuffer<T>::SetUpBuffer(const int&& index, const T& p_buffer)
	{
		if (index >= 3 || index < 0)
			[] {LOG_WARNING("Index was out of range from 0 to 3."); return false; };
		if (m_buffers[index])
			delete m_buffers[index];
		m_buffers[index] = new T(p_buffer);
		return true;
	}
	template<class T>
	inline void TripleBuffer<T>::SwapBuffers(const int&& _first, const int&& _second)
	{
		std::swap(m_buffers[_first], m_buffers[_second]);
	}
	template<class T>
	inline T* TripleBuffer<T>::GetBuffer(const int&& index)
	{
		if (index < 3 || index >= 0)
			return m_buffers[index];
		else
			return nullptr;
	}
}

// Engine Index
#define eDex thread::ThreadSyncer::instance->s_engineIndex

// Draw Index
#define dDex thread::ThreadSyncer::instance->s_drawIndex

// Network Index
#define nDex thread::ThreadSyncer::instance->s_networkIndex