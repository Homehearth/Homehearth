#pragma once

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)


namespace Logger
{
	constexpr int RED = 12;
	constexpr int GREEN = 10;
	constexpr int YELLOW = 14;
	constexpr int WHITE = 15;
	
	inline void SetConsoleTextColor(int color)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	}
	
} // end of namespace


// These are the macros to use if you want to log something to the console.
// Note: only works during debug mode, else the macros expand into doing nothing.
#ifdef _DEBUG
#define LOG_ERROR(...) Logger::SetConsoleTextColor(Logger::RED); printf("[ERROR: %s, line %d] ", __FILENAME__, __LINE__); Logger::SetConsoleTextColor(Logger::WHITE); printf(__VA_ARGS__); printf("\n");
#define LOG_WARNING(...) Logger::SetConsoleTextColor(Logger::YELLOW); printf("[WARNING: %s, line %d] ", __FILENAME__, __LINE__); Logger::SetConsoleTextColor(Logger::WHITE); printf(__VA_ARGS__); printf("\n");
#define LOG_INFO(...) Logger::SetConsoleTextColor(Logger::GREEN); printf("[INFO: %s, line %d] ", __FILENAME__, __LINE__); Logger::SetConsoleTextColor(Logger::WHITE); printf(__VA_ARGS__); printf("\n"); 
#define LOG_NETWORK(...) Logger::SetConsoleTextColor(FOREGROUND_BLUE); printf("[NETWORK: %s, line %d] ", __FILENAME__, __LINE__); Logger::SetConsoleTextColor(Logger::WHITE); printf(__VA_ARGS__); printf("\n");
#elif SERVER_LOG
#define LOG_ERROR(...) Logger::SetConsoleTextColor(Logger::RED); printf("[ERROR: %s, line %d] ", __FILENAME__, __LINE__); Logger::SetConsoleTextColor(Logger::WHITE); printf(__VA_ARGS__); printf("\n");
#define LOG_WARNING(...) Logger::SetConsoleTextColor(Logger::YELLOW); printf("[WARNING: %s, line %d] ", __FILENAME__, __LINE__); Logger::SetConsoleTextColor(Logger::WHITE);	printf(__VA_ARGS__); printf("\n");
#define LOG_INFO(...) Logger::SetConsoleTextColor(Logger::GREEN); printf("[INFO: %s, line %d] ", __FILENAME__, __LINE__); Logger::SetConsoleTextColor(Logger::WHITE); printf(__VA_ARGS__); printf("\n");
#define LOG_NETWORK(...) Logger::SetConsoleTextColor(FOREGROUND_BLUE); printf("[NETWORK: %s, line %d] ", __FILENAME__, __LINE__); Logger::SetConsoleTextColor(Logger::WHITE); printf(__VA_ARGS__); printf("\n");
#else
#define LOG_ERROR(...) Logger::SetConsoleTextColor(Logger::RED); printf("[ERROR: %s, line %d] ", __FILENAME__, __LINE__); Logger::SetConsoleTextColor(Logger::WHITE); printf(__VA_ARGS__); printf("\n");
#define LOG_WARNING(...) Logger::SetConsoleTextColor(Logger::YELLOW); printf("[WARNING: %s, line %d] ", __FILENAME__, __LINE__); Logger::SetConsoleTextColor(Logger::WHITE);	printf(__VA_ARGS__); printf("\n");
#define LOG_INFO(...) Logger::SetConsoleTextColor(Logger::GREEN); printf("[INFO: %s, line %d] ", __FILENAME__, __LINE__); Logger::SetConsoleTextColor(Logger::WHITE); printf(__VA_ARGS__); printf("\n");
#define LOG_NETWORK(...) Logger::SetConsoleTextColor(FOREGROUND_BLUE); printf("[NETWORK: %s, line %d] ", __FILENAME__, __LINE__); Logger::SetConsoleTextColor(Logger::WHITE); printf(__VA_ARGS__); printf("\n");
//#define LOG_ERROR(...)
//#define LOG_WARNING(...)
//#define LOG_INFO(...)
//#define LOG_NETWORK
#endif																																


/*	----------------------------------------------------------------------
 *		Other useful Visual C++ macros that may be used when logging:
 *	----------------------------------------------------------------------
 *
 *	__FUNCTION__					Holds the name of the function where it's called. 
 *	__TIME__ or __TIMESTAMP__		Holds the current time or date. 
 *	__FILE__						The pathname of the file from which the module was loaded.
 *	__LINE__						Current line number in the source file.
 *	
 */ 
