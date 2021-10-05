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
	srand(static_cast<unsigned>(time(NULL)));

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
	rtd::Handler2D::Initialize();
	BackBuffer::Initialize();

	//Camera
	Camera m_debugCamera;
	m_debugCamera.Initialize(sm::Vector3(0, 0, 1), sm::Vector3(0, 0, 0), sm::Vector3(0, 1, 0), sm::Vector2((float)m_window.GetWidth(), (float)m_window.GetHeight()));

	m_currentCamera = std::make_shared<Camera>(m_debugCamera);

	m_renderer.Initialize(&m_window, m_currentCamera.get());

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

	InputSystem::Get().SetMouseWindow(m_window.GetHWnd());

	m_client.Connect("127.0.0.1", 4950);

#if DRAW_TEMP_2D
	rtd::Button* test = new rtd::Button("demo_start_game_button.png", draw_t(100.0f, 100.0f, 275.0f, 100.0f), true);
	rtd::Button* test2 = new rtd::Button("demo_options_button.png", draw_t(100.0f, 225.0f, 275.0f, 100.0f), true);
	rtd::Button* test3 = new rtd::Button("demo_exit_button.png", draw_t(100.0f, 350.0f, 275.0f, 100.0f), false);
	rtd::Text* test4 = new rtd::Text("Welcome to Homehearth!", draw_text_t(350.0f, 25.0f, 300.0f, 100.0f));
	test->GetBorder()->SetColor(D2D1::ColorF(1.0f, 0.0f, 0.5f));
	test2->GetBorder()->SetColor(D2D1::ColorF(0.1f, .75f, 0.25f));
	test3->GetBorder()->SetColor(D2D1::ColorF(0.5f, .23f, 0.65f));
	test->SetName("Button1");
	rtd::Handler2D::InsertElement(test);
	rtd::Handler2D::InsertElement(test2);
	rtd::Handler2D::InsertElement(test3);
	rtd::Handler2D::InsertElement(test4);
#endif
}

void Engine::Run()
{

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
			if (GetForegroundWindow() == this->m_window.GetHWnd())
			{
				key[0] = GetAsyncKeyState('1') & 0x8000;
				key[1] = GetAsyncKeyState('2') & 0x8000;
				key[2] = GetAsyncKeyState('3') & 0x8000;

				if (key[0] && !old_key[0])
				{
					m_client.PingServer();
				}
				else if (key[1] && !old_key[1])
				{
					m_client.TestServerWithGibberishData();
				}

				for (int i = 0; i < 3; i++)
				{
					old_key[i] = key[i];
				}
			}
		}

		// Handle Input.
		InputSystem::Get().UpdateEvents();
		
		{
			PROFILE_SCOPE("Update 2D Elements");
			rtd::Handler2D::Update();
		}
		
		//Showing examples of keyboard and mouse (THIS CODE SHOULD BE HANDLED SOMEWHERE ELSE (GAMEPLAY LOGIC))
		if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::G, KeyState::RELEASED))
		{
			std::cout << "G Released\n";
#if DRAW_TEMP_2D
			if(rtd::Handler2D::GetElement<rtd::Button>("Button1"))
				rtd::Handler2D::GetElement<rtd::Button>("Button1")->Release();
#endif
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

#if DRAW_TEMP_2D
		rtd::Handler2D::EraseAll();
#endif
	}

	s_engineRunning = false;
	// Wait for the rendering thread to exit its last render cycle and shutdown
#if _DEBUG
	// This is debug since it catches release in endless loop.
	while (!s_safeExit) {};
#endif
	
	IMGUI(
		// ImGUI Shutdown
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	);


	m_client.Disconnect();
    T_DESTROY();
    D2D1Core::Destroy();
	ResourceManager::Get().Destroy();
	rtd::Handler2D::Destroy();
	BackBuffer::Destroy();
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
	static int dots = 0;
	if (timer.GetElapsedTime<std::chrono::duration<float>>() > 0.5f)
	{
		fpsContainer.emplace_back((1 / m_frameTime.render));
		fpsUpdateContainer.emplace_back((1.0f / m_frameTime.update));
		ramUsageContainer.emplace_back((Profiler::GetRAMUsage() / (1024.f * 1024.f)));
		vRamUsageContainer.emplace_back((Profiler::GetVRAMUsage() / (1042.f * 1024.f)));
		timer.Start();
		dots = (dots + 1) % 4;
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
#if PROFILER
	static bool isRecProfileSession = false;
	if (!isRecProfileSession)
	{
		if (ImGui::Button("Record"))
		{
			isRecProfileSession = true;
			PROFILER_BEGIN_SESSION();
		}
		ImGui::SameLine();
		ImGui::Text("- Starts Profiler Session");
	}
	else
	{
		if (ImGui::Button("Stop Recording"))
		{
			isRecProfileSession = false;
			PROFILER_END_SESSION();
		}
		ImGui::SameLine();
		std::string loadingDots = "";
		for (int i = 0; i < dots; i++)
			loadingDots.append(".");
		ImGui::TextColored(ImColor(1.f, 0.f, 0.f, 1.0f), ("Recording" + loadingDots).c_str());
	}
#endif
	if (ImGui::CollapsingHeader("FPS"))
	{
		ImGui::PlotLines(("FPS: " + std::to_string(static_cast<size_t>(1 / m_frameTime.render))).c_str(), fpsContainer.data(), static_cast<int>(fpsContainer.size()), 0, nullptr, 0.0f, 144.0f, ImVec2(150, 50));
		ImGui::Spacing();
		ImGui::PlotLines(("Update FPS: " + std::to_string(static_cast<size_t>(1.0f / m_frameTime.update))).c_str(), fpsUpdateContainer.data(), static_cast<int>(fpsUpdateContainer.size()), 0, nullptr, 0.0f, 144.0f, ImVec2(150, 50));
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Memory"))
	{
		ImGui::PlotHistogram(("RAM: " + std::to_string(Profiler::GetRAMUsage() / (1024.f * 1024.f)) + " MB").c_str(), ramUsageContainer.data(), static_cast<int>(ramUsageContainer.size()), 0, nullptr, 0.0f, 500.0f, ImVec2(150, 75));
		ImGui::Spacing();
		ImGui::PlotHistogram(("VRAM: " + std::to_string(Profiler::GetVRAMUsage() / (1024.f * 1024.f)) + " MB").c_str(), vRamUsageContainer.data(), static_cast<int>(vRamUsageContainer.size()), 0, nullptr, 0.0f, 500.0f, ImVec2(150, 75));
	}

	ImGui::End();
	
	ImGui::Begin("Components");
	if (ImGui::CollapsingHeader("Transform"))
	{
		m_currentScene->GetRegistry().view<comp::Transform>().each([&](entt::entity e, comp::Transform& transform)
			{
				ImGui::Separator();
				ImGui::Text("Entity: %d", static_cast<int>(e));
				ImGui::DragFloat3(("Position##" + std::to_string(static_cast<int>(e))).c_str(), (float*)&transform.position);
				ImGui::DragFloat3(("Rotation##" + std::to_string(static_cast<int>(e))).c_str(), (float*)&transform.rotation, dx::XMConvertToRadians(1.f));
				if(ImGui::Button(("Remove##" + std::to_string(static_cast<int>(e))).c_str()))
				{
					m_currentScene->GetRegistry().destroy(e);
				}
				ImGui::Spacing();
			});
	}
	ImGui::End();

	ImGui::Begin("Camera");
	{
		const std::string position = "Position: " + std::to_string(m_currentCamera->GetPosition().x)+ " " + std::to_string(m_currentCamera->GetPosition().y) + " " + std::to_string(m_currentCamera->GetPosition().z);
		ImGui::Separator();
		ImGui::Text(position.c_str());
		//ImGui::DragFloat("Zoom: ", &m_currentCamera->m_zoomValue, 0.01f, 0.f, 1.0f);
		ImGui::Spacing();

	};
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
			if (m_currentScene->IsRenderReady() && rtd::Handler2D::IsRenderReady())
			{
				Render(deltaSum);
				m_frameTime.render = deltaSum;
				deltaSum = 0.f;
			}
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
	{
		PROFILE_SCOPE("Starting ImGui");
		IMGUI(
			m_imguiMutex.lock();
			// Start ImGui frame
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		);
	}

	// Update elements in the scene.
	if (m_currentScene)
	{
		m_currentScene->Update(dt);
		m_currentCamera->Update(dt);

	}

	{
		PROFILE_SCOPE("Ending ImGui");

		IMGUI(
			drawImGUI();
			ImGui::EndFrame();
			m_imguiMutex.unlock();
		);
	}
}

void Engine::Render(float& dt)
{
	PROFILE_FUNCTION();

	/*
		Render 3D
	*/
	m_renderer.ClearFrame();
	m_renderer.Render(m_currentScene);

	{
		PROFILE_SCOPE("Render ImGui");
		IMGUI(
			m_imguiMutex.lock();
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			m_imguiMutex.unlock();
		);
	}

	{
		PROFILE_SCOPE("Render D2D1");
		D2D1Core::Begin();
		rtd::Handler2D::Render();
		D2D1Core::Present();
	}

	{
		PROFILE_SCOPE("Present");
		D3D11Core::Get().SwapChain()->Present(0, 0);
	}
}

