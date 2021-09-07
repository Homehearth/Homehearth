#include "EnginePCH.h"
#include "Profiler.h"

PROCESS_MEMORY_COUNTERS Profiler::GetMemoryInfo()
{
	PROCESS_MEMORY_COUNTERS pmc;
	if (FAILED(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))))
	{
		LOG_ERROR("Failed to get memory information");
	}
	return pmc;
}

size_t Profiler::GetRAMUsage() {
	return GetMemoryInfo().PagefileUsage;
}

size_t Profiler::GetVRAMUsage()
{
	return D3D11Core::Get().GetVideoMemoryInfo().CurrentUsage;
}
