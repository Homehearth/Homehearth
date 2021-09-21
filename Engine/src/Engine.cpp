#include "EnginePCH.h"
#include "Engine.h"
#include <omp.h>
#include "BackBuffer.h"

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
	Backbuffer::Initialize();


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
	//m_renderer.SetPipelineState();
	debugCamera.Preset();

	/*ResourceManager::GetResource<RMesh>("Monster.fbx");
	ResourceManager::GetResource<RMesh>("Cube.fbx");
	ResourceManager::GetResource<RMesh>("geo_house2.obj");*/

	//meshLOLXD = new MeshRenderObject(ResourceManager::GetResource<RMesh>("Cube.fbx"));

	// Thread should be launched after s_engineRunning is set to true and D3D11 is initalized.
	s_engineRunning = true;

	if (m_client.Connect("127.0.0.1", 4950))
	{
		LOG_INFO("Connected to server");
	}
	else {
		LOG_ERROR("Failed to connect to server");
	} 
    //m_client = std::make_unique<Client>();

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

	
	std::cout << "All clients mothafocking did it!" << std::endl;
}

void Engine::Run()
{
	double currentFrame = 0.f;
	double lastFrame = omp_get_wtime();
	float deltaTime = 0.f;
	float accumulator = 0.f;
	const float targetDelta = 1 / 1000.0f;

	if (thread::IsThreadActive())
		T_CJOB(Engine, RenderThread);
	
	MSG msg = { nullptr };
	while (IsRunning())
	{
		// Service any and all pending Windows messages.
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				Shutdown();
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
	while (!s_safeExit) {};

	// ImGUI Shutdown
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	
    T_DESTROY();
    ResourceManager::Destroy();
    D2D1Core::Destroy();
	Backbuffer::Destroy();
	//delete meshLOLXD;
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
	
	if(timer.getElapsedTime() > 0.5f)
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
		ImGui::PlotLines(("FPS: " + std::to_string(static_cast<int>(1 / m_frameTime.render))).c_str(), fpsContainer.data(), fpsContainer.size(), 0, nullptr, 0.0f, 144.0f, ImVec2(150, 50));
		ImGui::Spacing();
		ImGui::PlotLines(("Update FPS: " + std::to_string(static_cast<int>(1.0f / m_frameTime.update))).c_str(), fpsUpdateContainer.data(), fpsUpdateContainer.size(), 0, nullptr, 0.0f, 144.0f, ImVec2(150, 50));
		ImGui::Spacing();
	}

	if(ImGui::CollapsingHeader("Memory"))
	{
		ImGui::PlotHistogram(("RAM: "+std::to_string(static_cast<float>(Profiler::GetRAMUsage() / (1024.f * 1024.f))) + " MB").c_str(), ramUsageContainer.data(), ramUsageContainer.size(), 0, nullptr, 0.0f, 500.0f, ImVec2(150, 75));
		ImGui::Spacing();
		ImGui::PlotHistogram(("VRAM: " + std::to_string(static_cast<float>(Profiler::GetVRAMUsage() / (1024.f * 1024.f))) + " MB").c_str(), vRamUsageContainer.data(), vRamUsageContainer.size(), 0, nullptr, 0.0f, 500.0f, ImVec2(150, 75));
	}

	ImGui::End();
	
}

void Engine::RenderThread()
{
	double currentFrame = 0.f, lastFrame = omp_get_wtime();
	float deltaTime = 0.f, deltaSum = 0.f;
	// Desired FPS
	const float targetDelta = 1 / 144.01f;
	while (IsRunning())
	{
		currentFrame = omp_get_wtime();
		deltaTime = static_cast<float>(currentFrame - lastFrame);
		if (deltaSum >= targetDelta)
		{
			if(Backbuffer::GetBuffers()->IsSwapped())
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
	// Update the camera transform based on interactive inputs.
	//updateCamera(dt);

	// Update positions, orientations and any other
	// relevant visual state of any dynamic elements
	// in the scene.
	if (m_currentScene)
	{
		m_currentScene->Update(dt);
	}
	
	
	/*
#ifdef _DEBUG
	if(!m_IsImguiReady.load())
	{
		//Start ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		drawImGUI();
		
		m_IsImguiReady = true;
	}
#endif // DEBUG
*/

	
	//std::cout << "Y: " << y++ << "\n";
	// Handle events enqueued
	//Scene::GetEventDispatcher().update();

	/*
	Backbuffer::GetBuffers()->GetBuffer(0)->clear();
	Backbuffer::GetBuffers()->GetBuffer(0)->push_back(meshLOLXD);

	if (!Backbuffer::GetBuffers()->IsSwapped())
	{
		Backbuffer::GetBuffers()->SwapBuffers();
	}
	*/
}

void Engine::Render(float& dt)
{
	m_renderer.BeginFrame();
	//D2D1Core::Begin();
	m_renderer.ClearScreen();
	m_renderer.SetPipelineState();
	if (m_currentScene)
	{
		m_currentScene->Render();
	}

	

	debugCamera.UpdateDOOM();
	debugCamera.Render();

	//Backbuffer::GetBuffers()->GetBuffer(1)->at(0)->Render();
	//meshLOLXD->Render(objectPass::RenderPass::DEFAULT);

	/*
		Present the final image and clear it for next frame.
	*/

	/*
		Kanske v�nta p� att uppdateringstr�den kan swappa buffrar.
	*/
	//D2D1Core::Present();

	/*
#ifdef _DEBUG
	if (m_IsImguiReady.load())
	{
		// Assemble togheter draw data
		ImGui::Render();
		// Render draw data
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		m_IsImguiReady = false;
	}
#endif
//*/
	
	HRESULT hr = D3D11Core::Get().SwapChain()->Present(1, 0);
	if (FAILED(hr))
		std::cout << GetLastError() << "\n";

	Backbuffer::GetBuffers()->ReadySwap();
}

