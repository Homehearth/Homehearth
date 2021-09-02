
class Logger
{
public:
	virtual ~Logger() = default;

	static auto& Get(){
		static Logger instance;
		return instance;
	}

	template<typename... Args>
	static void LogInfo(Args... args) {
		std::string str;
		auto l = [](std::string& str, auto val) {
			if constexpr (std::is_arithmetic_v<decltype(val)>)
				str += std::to_string(val);
			else
				str += val;
		};
		(l(str, args), ...);
		LogToMessageBox("Info", str);
	}

	template<typename... Args>
	static void LogWarning(Args... args) {
		std::string str;
		auto l = [](std::string& str, auto val) {
			if constexpr (std::is_arithmetic_v<decltype(val)>)
				str += std::to_string(val);
			else
				str += val;
		};
		(l(str, args), ...);
		LogToMessageBox("Warning", str);
	}

	template<typename... Args>
	static void LogError(Args... args) {
		std::string str;
		auto l = [](std::string& str, auto val) {
			if constexpr (std::is_arithmetic_v<decltype(val)>)
				str += std::to_string(val);
			else
				str += val;
		};
		(l(str, args), ...);
		LogToMessageBox("Error",str);
	}
	
private:
	Logger() = default;

	static void LogToMessageBox(const std::string& level, const std::string& message) {
		MessageBoxA(nullptr, message.c_str(), level.c_str(), 0);
	}
};

#define LOG_INFO(...) Logger::Get().LogInfo( __FILE__, "(", __LINE__, "): [INFO] ", __FUNCTION__, ": ", __VA_ARGS__ )
#define LOG_WARNING(...) Logger::Get().LogWarning( __FILE__, "(", __LINE__, "): [WARNING] ", __FUNCTION__, ": ", __VA_ARGS__ )
#define LOG_ERROR(...) Logger::Get().LogError( __FILE__, "(", __LINE__, "): [ERROR] ", __FUNCTION__, ": ", __VA_ARGS__ )