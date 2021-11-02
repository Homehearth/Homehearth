#pragma once

class Profiler 
{
private:
	std::ofstream m_outputStream;
	int m_profileCount;

	std::mutex m_mutex;
public:
	struct Result
	{
		std::string name;
		long long start, end;
		uint32_t threadID;
		Result(): start(0), end(0), threadID(0){} ;
	};

	class ProfileTimer
	{
	private:
		Timer m_timer;
		std::string m_name;
	public:
		ProfileTimer(const std::string& name);
		~ProfileTimer();

	};

	Profiler();

	static Profiler& Get()
	{
		static Profiler instance;
		return instance;
	}

	static PROCESS_MEMORY_COUNTERS GetMemoryInfo();
	static size_t GetRAMUsage();
	static size_t GetVRAMUsage();


	void BeginSession(const std::string& filepath = "profile_result.json");

	void EndSession();

	void WriteHeader();
	
	void WriteProfile(const Profiler::Result& result);
	
	void WriteFooter();


};



#if PROFILER
#define PROFILER_BEGIN_SESSION_PATH(filepath) ::Profiler::Get().BeginSession(filepath)
#define PROFILER_BEGIN_SESSION() ::Profiler::Get().BeginSession()
#define PROFILER_END_SESSION() ::Profiler::Get().EndSession()
#define PROFILE_SCOPE(name) ::Profiler::ProfileTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCSIG__)
#else
#define PROFILER_BEGIN_SESSION(filepath)
#define PROFILER_END_SESSION()
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()
#endif

