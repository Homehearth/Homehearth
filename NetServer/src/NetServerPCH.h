#pragma once

// Windows
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsock2.h>
#include <windowsx.h>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.Lib")
#pragma comment(lib, "winmm.lib")
#endif

// Standard
#include <string>
#include <string_view>
#include <exception>
#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <io.h>
#include <memory>
#include <cassert> 
#include <unordered_map>
#include <vector>
#include <array>
#include <set>
#include <queue>


// Custom Global includes (Singletons)
#include "Logger.h"
#include <EnginePCH.h>

//const std::string BOUNDSPATH = "../../../../../Assets/Bounds/";
