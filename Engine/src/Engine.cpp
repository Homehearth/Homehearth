#include "EnginePCH.h"
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

#ifdef _DEBUG
	m_IsImguiReady = false;
	// Setup ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(m_window.GetHWnd());
	ImGui_ImplDX11_Init(D3D11Core::Get().Device(), D3D11Core::Get().DeviceContext());
	ImGui::StyleColorsDark();
	ImGui_ImplDX11_CreateDeviceObjects(); // uses device, therefore has to be called before render thread starts
	LOG_INFO("ImGui was successfully initialized");
#endif
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
			InputSystem::Get().UpdateEvents();
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
		// todo:


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
	while (!s_safeExit) {}; // TODO: why only in debug??

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
	
	if(timer.GetElapsedTime() > 0.5f)
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

	if(ImGui::CollapsingHeader("Memory"))
	{
		ImGui::PlotHistogram(("RAM: "+std::to_string(static_cast<float>(Profiler::GetRAMUsage() / (1024.f * 1024.f))) + " MB").c_str(), ramUsageContainer.data(), static_cast<int>(ramUsageContainer.size()), 0, nullptr, 0.0f, 500.0f, ImVec2(150, 75));
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
	const float targetDelta = 1 / 144.01f; 	// Desired FPS
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
	// todo:
	// Update the camera transform based on interactive inputs.

	// Update elements in the scene.
	if (m_currentScene)
	{
		m_currentScene->Update(dt);
	}
	
#ifdef _DEBUG
	if(!m_IsImguiReady)
	{
		// Start ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		drawImGUI();
		
		ImGui::EndFrame();
		
		m_IsImguiReady = true;

	}
#endif // DEBUG

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

	D2D1Core::Present();

#ifdef _DEBUG
	if (m_IsImguiReady)
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		m_IsImguiReady = false;
	}
#endif
	
	D3D11Core::Get().SwapChain()->Present(0, 0);
}

