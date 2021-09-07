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
}

// Engine Index
#define eDex thread::ThreadSyncer::instance->s_engineIndex

// Draw Index
#define dDex thread::ThreadSyncer::instance->s_drawIndex

// Network Index
#define nDex thread::ThreadSyncer::instance->s_networkIndex