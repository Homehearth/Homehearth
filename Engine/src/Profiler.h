#pragma once

namespace Profiler 
{

	PROCESS_MEMORY_COUNTERS GetMemoryInfo();
	size_t GetRAMUsage();
	size_t GetVRAMUsage();

};