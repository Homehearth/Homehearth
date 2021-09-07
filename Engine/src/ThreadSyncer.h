#pragma once

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
	static unsigned int s_engineIndex;
	static unsigned int s_drawIndex;
	static unsigned int s_networkIndex;

public:

	static void Initialize();
	static void Destroy();

};