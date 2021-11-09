#pragma once

//Options
#define RENDER_IMGUI 0
#define PROFILER 1

// Turn this to 1 if you want to see the very beautiful demo menu
#define DRAW_TEMP_2D 0
#define DEBUG_SNAPSHOT 0

#define USE_MIPMAPS 1
#define RENDER_GRID 0 

//Macros
#if RENDER_IMGUI
#define IMGUI(a) do {a} while(0)
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
#include <condition_variable>
#include <bitset>

#include <functional>

#define ALIGN16 __declspec(align(16)) 

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
#include "CollisionSystem.h"


// imGUI
#include <imgui.h>
#include <imconfig.h>
#include <imgui_internal.h>
#include <imstb_rectpack.h>
#include <imstb_textedit.h>
#include <imstb_truetype.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

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
#include "Components.h"
#include "SceneBuilder.h"
#include "RenderThreadHandler.h"
#include "Stats.h"

// Network
#include "Client.h"

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

// Paths
const std::string MODELPATH		= "../Assets/Models/";		//"../../../../../Assets/Models/"
const std::string MATERIALPATH	= "../Assets/Materials/";
const std::string TEXTUREPATH	= "../Assets/Textures/";
const std::string ANIMATIONPATH = "../Assets/Animations/";
const std::string ANIMATORPATH	= "../Assets/Animators/";
const std::string FONTPATH		= "../Assets/Fonts/";
