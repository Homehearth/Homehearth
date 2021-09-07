#pragma once

class Profiler 
{
private:
	
public:
	static Profiler& Get() 
	{
		static Profiler instance;
		return instance;
	}

	PROCESS_MEMORY_COUNTERS GetMemoryInfo();
	size_t GetRAMUsage();
	size_t GetVRAMUsage();

};