#pragma once

// Windows
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <windowsx.h>
#pragma comment(lib, "winmm.lib")
#define HInstance() GetModuleHandle(NULL)
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


// DirectX
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl/client.h>
#include <dwrite.h>
#include <dwrite_3.h>
#include <d2d1.h>
#include <ctime>


// DirectXTK
#include "SimpleMath.h"
#include "BufferHelpers.h"
#include <Audio.h>


// imGUI
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"


// Custom
#include "Logger.h"
#include "multi_thread_manager.h"
#include "ResourceManager.h"

using Microsoft::WRL::ComPtr;