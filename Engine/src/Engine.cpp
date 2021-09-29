#include "EnginePCH.h"
#include "Engine.h"
#include <omp.h>
#include "Camera.h"

#include "RMesh.h"

bool Engine::s_engineRunning = false;
bool Engine::s_safeExit = false;

Engine::Engine()
	: m_scenes({ 0 })
	, m_currentScene(nullptr)
	, m_vSync(false)
	, m_frameTime()
{
	LOG_INFO("Engine(): " __TIMESTAMP__);
}

void Engine::Startup()
{
	T_INIT(1, thread::ThreadType::POOL_FIFO);
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
	m_currentcamera = std::make_shared<Camera>();
	//Camera
	m_currentcamera->Initialize(sm::Vector3(0, 0, -1), sm::Vector3(0, 0, 0), sm::Vector3(0, 1, 0), sm::Vector2((float)m_window.GetWidth(), (float)m_window.GetHeight()));

	m_renderer.Initialize(&m_window, m_currentcamera.get());

	// Thread should be launched after s_engineRunning is set to true and D3D11 is initialized.
	s_engineRunning = true;

	

	//
	// AUDIO 
	//
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to initialize AudioEngine.");
	}
	DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default;
#ifdef _DEBUG
	eflags |= DirectX::AudioEngine_Debug;
#endif
	this->m_audio_engine = std::make_unique<DirectX::AudioEngine>(eflags);


	IMGUI(
		// Setup ImGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO & io = ImGui::GetIO();
		ImGui_ImplWin32_Init(m_window.GetHWnd());
		ImGui_ImplDX11_Init(D3D11Core::Get().Device(), D3D11Core::Get().DeviceContext());
		ImGui::StyleColorsDark();
		ImGui_ImplDX11_CreateDeviceObjects(); // uses device, therefore has to be called before render thread starts
		LOG_INFO("ImGui was successfully initialized");
	);

	InputSystem::Get().SetMouseWindow(m_window.GetHWnd(), m_window.GetWidth(), m_window.GetHeight());
	InputSystem::Get().SetCamera(m_currentcamera.get());

	m_client.Connect("127.0.0.1", 4950);

	
	
}

void Engine::Run()
{

	PROFILER_BEGIN_SESSION();

	double currentFrame = 0.f;
	double lastFrame = omp_get_wtime();
	float deltaTime = 0.f;
	float accumulator = 0.f;
	const float targetDelta = 1 / 10000.0f;

	bool key[3] = { false, false, false };
	bool old_key[3] = { false, false, false };

	if (thread::IsThreadActive())
		T_CJOB(Engine, RenderThread);

	MSG msg = { nullptr };
	while (IsRunning())
	{
		PROFILE_SCOPE("Frame");
		InputSystem::Get().UpdateEvents();

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
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
		
		m_currentcamera->Update(deltaTime);
		
		//Showing examples of keyboard and mouse (THIS CODE SHOULD BE HANDLED SOMEWHERE ELSE (GAMEPLAY LOGIC))
		if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::G, KeyState::RELEASED))
		{
			std::cout << "G Released\n";
		}
		if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
		{
			std::cout << "Mouse left Pressed\n";
			std::cout << "XPos: " << InputSystem::Get().GetMousePos().x << std::endl;
			//Testing mouse ray TODO Move this update to game?
			InputSystem::Get().UpdateMouseRay();
			std::cout << "Mouseraydir: " << InputSystem::Get().GetMouseRay().rayDir.x << " " << InputSystem::Get().GetMouseRay().rayDir.y << " " << InputSystem::Get().GetMouseRay().rayDir.z << std::endl;
			//LOG_INFO("Mouseray: " + std::to_string(InputSystem::Get().GetMouseRay().rayDir.x));
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
		//if (InputSystem::Get().GetAxis(Axis::HORIZONTAL) == 1)
		//{
		//	std::cout << "Moving right\n";
		//}

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


	// Wait for the rendering thread to exit its last render cycle and shutdown
	IMGUI(
		while (!s_safeExit) {}; // TODO: why only in debug??
		// ImGUI Shutdown
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	);

	m_client.Disconnect();
    T_DESTROY();
    ResourceManager::Get().Destroy();
    D2D1Core::Destroy();

	PROFILER_END_SESSION();
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

	if (timer.GetElapsedTime() > 0.5f)
	{
		fpsContainer.emplace_back((1 / m_frameTime.render));
		fpsUpdateContainer.emplace_back((1.0f / m_frameTime.update));
		ramUsageContainer.emplace_back((Profiler::GetRAMUsage() / (1024.f * 1024.f)));
		vRamUsageContainer.emplace_back((Profiler::GetVRAMUsage() / (1042.f * 1024.f)));
		timer.Start();
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
	
	ImGui::Begin("Objects");
	m_currentScene->GetRegistry().view<comp::Transform>().each([&](entt::entity e, comp::Transform& transform)
		{

			ImGui::Separator();
			ImGui::DragFloat3(("Position: " + std::to_string((int)e)).c_str(), (float*)&transform.position);
			ImGui::DragFloat3(("Rotation: " + std::to_string((int)e)).c_str(), (float*)&transform.rotation, dx::XMConvertToRadians(1.f));
			ImGui::Spacing();

		});
	ImGui::End();
}

void Engine::RenderThread()
{
	double currentFrame = 0.f, lastFrame = omp_get_wtime();
	float deltaTime = 0.f, deltaSum = 0.f;
	const float targetDelta = 1 / 10000.0f; 	// Desired FPS
	while (IsRunning())
	{
		currentFrame = omp_get_wtime();
		deltaTime = static_cast<float>(currentFrame - lastFrame);
		if (deltaSum >= targetDelta)
		{
			Render(deltaSum);
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
	PROFILE_FUNCTION();
	
	// todo:
	// Update the camera transform based on interactive inputs.
	
	IMGUI(
		m_imguiMutex.lock();
		// Start ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	);

	// Update elements in the scene.
	if (m_currentScene)
	{
		m_currentScene->Update(dt);
	}

	IMGUI(
		drawImGUI();
		ImGui::EndFrame();
		m_imguiMutex.unlock();
	);

}

void Engine::Render(float& dt)
{
	PROFILE_FUNCTION();

	if (!m_currentScene->IsRenderReady())
		return;

	m_renderer.ClearFrame();
	m_renderer.Render();
	D2D1Core::Begin();
	if (m_currentScene)
	{
		m_currentScene->Render();
	}

	D2D1Core::Present();

	IMGUI(
		m_imguiMutex.lock();
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		m_imguiMutex.unlock();
	);

	D3D11Core::Get().SwapChain()->Present(0, 0);
}

