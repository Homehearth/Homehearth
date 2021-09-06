#pragma once


#pragma region Logger

class Logger
{
private:
	Logger()
	{
		LogSeperator();
	}
	
	static void LogToFile(const std::string& logText)
	{
		auto now = std::chrono::system_clock::now();
		time_t time = std::chrono::system_clock::to_time_t(now);
		char* ctime = std::ctime(&time);
		ctime[strlen(ctime) - 1] = '\0';

		std::fstream outfile;
		const std::string file = "server.log";
		outfile.open(file, std::ios_base::app);
		if (outfile.is_open()) {
			outfile << "[" << ctime << "]  " << logText + '\n';
			outfile.close();
		}
		else {
			const std::string msg = "Unable to open " + file;
			MessageBoxA(nullptr, msg.c_str(), "Error", 0);
		}
	}

	static void LogSeperator()
	{
		std::fstream outfile;
		const std::string file = "server.log";
		outfile.open(file, std::ios_base::app);
		if (outfile.is_open()) {
			outfile << "\n";
			for (int i = 0; i < 130; i++)
				outfile << "-";
			outfile << "\n\n";
			outfile.close();
		}
		else {
			const std::string msg = "Unable to open " + file;
			MessageBoxA(nullptr, msg.c_str(), "Error", 0);
		}
	}

public:
	virtual ~Logger() = default;

	static auto& Get()
	{
		static Logger s_instance;
		return s_instance;
	}

	template<typename... Args>
	static void LogInfo(Args... args)
	{
		std::string str;
		auto l = [](std::string& str, auto val) {
			if constexpr (std::is_arithmetic_v<decltype(val)>)
				str += std::to_string(val);
			else
				str += val;
		};
		(l(str, args), ...);
		LogToFile(str);
	}

	template<typename... Args>
	static void LogWarning(Args... args)
	{
		std::string str;
		auto l = [](std::string& str, auto val) {
			if constexpr (std::is_arithmetic_v<decltype(val)>)
				str += std::to_string(val);
			else
				str += val;
		};
		(l(str, args), ...);
		LogToFile(str);
	}

	template<typename... Args>
	static void LogError(Args... args)
	{
		std::string str;
		auto l = [](std::string& str, auto val) {
			if constexpr (std::is_arithmetic_v<decltype(val)>)
				str += std::to_string(val);
			else
				str += val;
		};
		(l(str, args), ...);
		LogToFile(str);
	}

	template<typename... Args>
	static void LogToConsole(Args... args)
	{
		std::string str;
		auto l = [](std::string& str, auto val) {
			if constexpr (std::is_arithmetic_v<decltype(val)>)
				str += std::to_string(val);
			else
				str += val;
		};
		(l(str, args), ...);
		std::cout << str << std::endl;
	}

	
	// No copying allowed.
	Logger(const Logger& other) = delete;
	Logger& operator=(const Logger& other) = delete;	
};

#pragma endregion

// Log to file macros.
#define LOG_INFO(...) Logger::Get().LogInfo( __FILE__, "(", __LINE__, "): [INFO] ", __FUNCTION__, ": ", __VA_ARGS__ )
#define LOG_WARNING(...) Logger::Get().LogWarning( __FILE__, "(", __LINE__, "): [WARNING] ", __FUNCTION__, ": ", __VA_ARGS__ )
#define LOG_ERROR(...) Logger::Get().LogError( __FILE__, "(", __LINE__, "): [ERROR] ", __FUNCTION__, ": ", __VA_ARGS__ )

// Log to console macro.
#define LOG_CONSOLE(...) Logger::Get().LogToConsole( __VA_ARGS__ )