#include "EnginePCH.h"
#include "Profiler.h"

Profiler::Profiler()
	: m_profileCount(0)
{
}

PROCESS_MEMORY_COUNTERS Profiler::GetMemoryInfo()
{
	PROCESS_MEMORY_COUNTERS pmc;
	if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
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

void Profiler::BeginSession(const std::string& filepath)
{
	if (m_outputStream.is_open())
		return;

	m_mutex.lock();

	m_outputStream.open(filepath);
	if (!m_outputStream.is_open())
	{
		LOG_ERROR("Failed to open session file");
		m_mutex.unlock();
		return;
	}
	WriteHeader();

	m_mutex.unlock();
}

void Profiler::EndSession()
{
	if (!m_outputStream.is_open())
		return;

	m_mutex.lock();

	WriteFooter();
	m_outputStream.close();
	m_profileCount = 0;

	m_mutex.unlock();
}

void Profiler::WriteHeader()
{
	m_outputStream << "{\"otherData\": {}, \"traceEvents\":[";
	m_outputStream.flush();
}

void Profiler::WriteProfile(const Profiler::Result& result)
{
	if (!m_outputStream.is_open())
		return;

	m_mutex.lock();

	if (m_profileCount++ > 0)
	{
		m_outputStream << ",";
	}

	std::string name = result.name;
	std::replace(name.begin(), name.end(), '"', '\'');

	m_outputStream << "{";
	m_outputStream << "\"cat\":\"function\",";
	m_outputStream << "\"dur\":" << (result.end - result.start) << ',';
	m_outputStream << "\"name\":\"" << name << "\",";
	m_outputStream << "\"ph\":\"X\",";
	m_outputStream << "\"pid\":0,";
	m_outputStream << "\"tid\":" << result.threadID << ",";
	m_outputStream << "\"ts\":" << result.start;
	m_outputStream << "}";

	m_outputStream.flush();

	m_mutex.unlock();
}

void Profiler::WriteFooter()
{
	m_outputStream << "]}";
	m_outputStream.flush();
}

Profiler::ProfileTimer::ProfileTimer(const std::string& name)
	: m_name(name)
{
	m_timer.Start();
}

Profiler::ProfileTimer::~ProfileTimer()
{
	m_timer.Stop();
	Profiler::Result res;
	res.start = m_timer.GetStartTime<std::chrono::microseconds>();
	res.end = m_timer.GetStopTime<std::chrono::microseconds>();
	res.name = m_name;
	res.threadID = uint32_t(std::hash<std::thread::id>()(std::this_thread::get_id()));
	Profiler::Get().WriteProfile(res);
}
