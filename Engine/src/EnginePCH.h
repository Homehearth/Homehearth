#pragma once
#pragma warning(push, 3)

#define RENDER_IMGUI 1
#define PROFILER 0

//Macros
#if RENDER_IMGUI
#define IMGUI(a) a
#else
#define IMGUI(a) do {} while(0)
#endif


// Windows
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wincodec.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.Lib")
#pragma comment(lib, "msvcrtd.lib")
#include <windowsx.h>
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


#include <functional>


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
#include <Keyboard.h>
#include <Mouse.h>
using Microsoft::WRL::ComPtr;
namespace dx = DirectX;


// DirectXTK
#include <SimpleMath.h>
#include <BufferHelpers.h>
#include <Audio.h>
#include <Keyboard.h>
#include <Mouse.h>
namespace sm = dx::SimpleMath;

// entt (ECS library)
#include <entt.hpp>

//Entity systems
#include "Systems.h"

// imGUI
#include <imgui.h>
#include <imconfig.h>
#include <imgui_internal.h>
#include <imstb_rectpack.h>
#include <imstb_textedit.h>
#include <imstb_truetype.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#pragma warning(pop)

//Utility
#include "Timer.h"

// Custom Global includes (Singletons)
#include "Logger.h"
#include "multi_thread_manager.h"
#include "ResourceManager.h"
#include "InputSystem.h"
#include "D3D11Core.h"
#include "D2D1Core.h"
#include "Profiler.h"
#include "ThreadSyncer.h"
#include "Handler2D.h"

// Network
#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_client_interface.h"

// Assimp
#pragma warning(push, 0)
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#pragma warning(pop)

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)