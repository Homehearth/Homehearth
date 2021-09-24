﻿#include "EnginePCH.h"
#include "Engine.h"
#include <omp.h>

#include "RMesh.h"

bool Engine::s_engineRunning = false;
bool Engine::s_safeExit = false;

Engine::Engine()
	: m_scenes({ 0 })
	, m_currentScene(nullptr)
	, m_vSync(false)
	, m_frameTime()
	, m_buffPointer(nullptr)
{
	LOG_INFO("Engine(): " __TIMESTAMP__);
}

void Engine::Startup()
{
	T_INIT(1, thread::ThreadType::POOL_FIFO);
	ResourceManager::Initialize();
	srand((unsigned int)time(NULL));

	// Window Startup:
	Window::Desc config;
	config.title = L"Engine";
	if (!m_window.Initialize(config))
	{
		LOG_ERROR("Could not Initialize m_window.");
	}

	// DirectX Startup:
	D3D11Core::Get().Initialize(&m_window);
	D2D1Core::Initialize(&m_window);

	m_renderer.Initialize(&m_window);

	// Thread should be launched after s_engineRunning is set to true and D3D11 is initialized.
	s_engineRunning = true;

	// Preallocate space for Triplebuffer
	m_drawBuffers.AllocateBuffers();
	m_buffPointer = m_drawBuffers.GetBuffer(0);
	m_buffPointer->reserve(200);
	m_buffPointer = m_drawBuffers.GetBuffer(1);
	m_buffPointer->reserve(200);


#ifdef _DEBUG
	m_IsImguiReady = false;
	// Setup ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(m_window.GetHWnd());
	ImGui_ImplDX11_Init(D3D11Core::Get().Device(), D3D11Core::Get().DeviceContext());
	ImGui::StyleColorsDark();
	LOG_INFO("ImGui was successfully initialized");
#endif
	InputSystem::Get().SetMouseWindow(m_window.GetHWnd());

	m_client.Connect("127.0.0.1", 4950);
}

void Engine::Run()
{
	double currentFrame = 0.f;
	double lastFrame = omp_get_wtime();
	float deltaTime = 0.f;
	float accumulator = 0.f;
	const float targetDelta = 1 / 1000.0f;

	bool key[3] = { false, false, false };
	bool old_key[3] = { false, false, false };

	if (thread::IsThreadActive())
		T_CJOB(Engine, RenderThread);

	MSG msg = { nullptr };
	while (IsRunning())
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				Shutdown();
			}
		}

		if (m_client.IsConnected())
		{
			if (!m_client.messages.empty())
			{
				std::cout << "TESTING!!!!!" << std::endl;
				message<MessageType> msg = m_client.messages.pop_front();
			}
			if (GetForegroundWindow() == this->m_window.GetHWnd())
			{
				key[0] = GetAsyncKeyState('1') & 0x8000;
				key[1] = GetAsyncKeyState('2') & 0x8000;
				key[2] = GetAsyncKeyState('3') & 0x8000;

				if (key[0] && !old_key[0])
				{
					message<MessageType> msg = {};
					msg.header.id = MessageType::PingServer;
					m_client.timeThen = std::chrono::system_clock::now();
					LOG_INFO("Pinging server!");

					m_client.Send(msg);
				}

				for (int i = 0; i < 3; i++)
				{
					old_key[i] = key[i];
				}
			}
		}

		// Handle Input.
		InputSystem::Get().UpdateEvents();
		// todo:
		if (InputSystem::Get().CheckKeyboardKey(DirectX::Keyboard::D, KeyState::RELEASED))
		{
			std::cout << "D Pressed\n";
		}
		if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
		{
			std::cout << "Mouse left Pressed\n";
			std::cout << "XPos: " << InputSystem::Get().GetMousePos().x << std::endl;
		}
		if (InputSystem::Get().CheckMouseKey(MouseKey::RIGHT, KeyState::PRESSED))
		{
			std::cout << "Switching mouse mode\n";
			InputSystem::Get().SwitchMouseMode();
		}
		if (InputSystem::Get().CheckMouseKey(MouseKey::MIDDLE, KeyState::PRESSED))
		{
			std::cout << "Toggling mouse visibility\n";
			InputSystem::Get().ToggleMouseVisibility();
		}

		// Update time.
		currentFrame = omp_get_wtime();
		deltaTime = static_cast<float>(currentFrame - lastFrame);

		if (accumulator >= targetDelta)
		{
			Update(accumulator);

			m_frameTime.update = accumulator;
			accumulator = 0.f;
		}
		accumulator += deltaTime;
		lastFrame = currentFrame;
	}


	// Wait for the rendering thread to exit its last render cycle and shutdown.
#ifdef _DEBUG
	while (!s_safeExit) {};

	// ImGUI Shutdown
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	m_client.Disconnect();
    T_DESTROY();
    ResourceManager::Destroy();
    D2D1Core::Destroy();
}

void Engine::Shutdown()
{
	s_engineRunning = false;
}

Scene& Engine::GetScene(const std::string& name)
{
	return m_scenes[name];
}

void Engine::SetScene(const std::string& name)
{
	SetScene(m_scenes.at(name));
}

void Engine::SetScene(Scene& scene)
{
	if (m_currentScene)
	{
		m_currentScene->clear();
	}
	m_currentScene = &scene;

	m_currentScene->on<EShutdown>([&](const EShutdown& e, Scene& scene)
	{
		Shutdown();
	});

	m_currentScene->on<ESceneChange>([&](const ESceneChange& e, Scene& scene)
	{
		SetScene(e.newScene);
	});
}

Window* Engine::GetWindow()
{
	return &m_window;
}

bool Engine::IsRunning()
{
	return s_engineRunning;
}

void Engine::drawImGUI() const
{
	//Containers for plotting
	static std::vector<float> fpsContainer;
	static std::vector<float> fpsUpdateContainer;
	static std::vector<float> ramUsageContainer;
	static std::vector<float> vRamUsageContainer;

	static Timer timer;

	if (timer.getElapsedTime() > 0.5f)
	{
		fpsContainer.emplace_back((1 / m_frameTime.render));
		fpsUpdateContainer.emplace_back((1.0f / m_frameTime.update));
		ramUsageContainer.emplace_back((Profiler::GetRAMUsage() / (1024.f * 1024.f)));
		vRamUsageContainer.emplace_back((Profiler::GetVRAMUsage() / (1042.f * 1024.f)));
		timer.start();
	}

	if (fpsContainer.size() > 10)
		fpsContainer.erase(fpsContainer.begin());

	if (fpsUpdateContainer.size() > 10)
		fpsUpdateContainer.erase(fpsUpdateContainer.begin());

	if (ramUsageContainer.size() > 10)
		ramUsageContainer.erase(ramUsageContainer.begin());

	if (vRamUsageContainer.size() > 10)
		vRamUsageContainer.erase(vRamUsageContainer.begin());

	ImGui::Begin("Window");
	const std::string screenRes = "Screen Resolution: " + std::to_string(m_window.GetWidth()) + "x" + std::to_string(m_window.GetHeight());
	ImGui::Text("%s", screenRes.c_str());
	ImGui::End();


	ImGui::Begin("Statistics");
	if (ImGui::CollapsingHeader("FPS"))
	{
		ImGui::PlotLines(("FPS: " + std::to_string(static_cast<size_t>(1 / m_frameTime.render))).c_str(), fpsContainer.data(), static_cast<int>(fpsContainer.size()), 0, nullptr, 0.0f, 144.0f, ImVec2(150, 50));
		ImGui::Spacing();
		ImGui::PlotLines(("Update FPS: " + std::to_string(static_cast<size_t>(1.0f / m_frameTime.update))).c_str(), fpsUpdateContainer.data(), static_cast<int>(fpsUpdateContainer.size()), 0, nullptr, 0.0f, 144.0f, ImVec2(150, 50));
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Memory"))
	{
		ImGui::PlotHistogram(("RAM: " + std::to_string(static_cast<float>(Profiler::GetRAMUsage() / (1024.f * 1024.f))) + " MB").c_str(), ramUsageContainer.data(), static_cast<int>(ramUsageContainer.size()), 0, nullptr, 0.0f, 500.0f, ImVec2(150, 75));
		ImGui::Spacing();
		ImGui::PlotHistogram(("VRAM: " + std::to_string(static_cast<float>(Profiler::GetVRAMUsage() / (1024.f * 1024.f))) + " MB").c_str(), vRamUsageContainer.data(), static_cast<int>(vRamUsageContainer.size()), 0, nullptr, 0.0f, 500.0f, ImVec2(150, 75));
	}

	ImGui::End();

}

void Engine::RenderThread()
{
	double currentFrame = 0.f, lastFrame = omp_get_wtime();
	float deltaTime = 0.f, deltaSum = 0.f;
	const float targetDelta = 1 / 144.01f; 	// Desired FPS
	while (IsRunning())
	{
		currentFrame = omp_get_wtime();
		deltaTime = static_cast<float>(currentFrame - lastFrame);
		if (deltaSum >= targetDelta)
		{
			if (m_drawBuffers.IsSwapped())
			{
				Render(deltaSum);
			}

			m_frameTime.render = deltaSum;
			deltaSum = 0.f;
		}
		deltaSum += deltaTime;
		lastFrame = currentFrame;

	}

	s_safeExit = true;
}

void Engine::Update(float dt)
{
	m_buffPointer = m_drawBuffers.GetBuffer(0);

	// Update the camera transform based on interactive inputs.
	// todo:

	// Update elements in the scene.
	if (m_currentScene)
	{
		m_currentScene->Update(dt);
	}


#ifdef _DEBUG
	if (!m_IsImguiReady.load())
	{
		// Start ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		drawImGUI();

		m_IsImguiReady = true;
	}
#endif // DEBUG

	if (!m_drawBuffers.IsSwapped())
	{
		m_drawBuffers.SwapBuffers();
	}
}

void Engine::Render(float& dt)
{
	m_renderer.ClearFrame();
	m_renderer.Render();
	D2D1Core::Begin();
	if (m_currentScene)
	{
		m_currentScene->Render();
	}

	m_drawBuffers.ReadySwap();
	D2D1Core::Present();

#ifdef _DEBUG
	if (m_IsImguiReady.load())
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		m_IsImguiReady = false;
	}
#endif

	D3D11Core::Get().SwapChain()->Present(1, 0);
}

