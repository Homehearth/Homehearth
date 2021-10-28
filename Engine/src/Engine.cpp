﻿#include "EnginePCH.h"
#include "Engine.h"
#include <omp.h>
#include "Camera.h"
#include "GridSystem.h"

bool Engine::s_safeExit = false;

Engine::Engine()
	: BasicEngine()
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
	BackBuffer::Initialize();

	m_renderer.Initialize(&m_window);

	// Thread should be launched after s_engineRunning is set to true and D3D11 is initialized.
	//
	// AUDIO - we supposed to use other audio engine
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

	// Thread Startup.
	thread::RenderThreadHandler::Get().SetRenderer(&m_renderer);
	thread::RenderThreadHandler::Get().SetWindow(&m_window);
	thread::RenderThreadHandler::Get().Setup(2);

	InputSystem::Get().SetMouseWindow(m_window.GetHWnd(), m_window.GetWidth(), m_window.GetHeight());

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
	
	BasicEngine::Startup();
}

void Engine::Run()
{
	if (thread::IsThreadActive())
		T_CJOB(Engine, RenderThread);

	BasicEngine::Run();
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

    T_DESTROY();
    D2D1Core::Destroy();
	ResourceManager::Get().Destroy();
	BackBuffer::Destroy();
}


Window* Engine::GetWindow()
{
	return &m_window;
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
		
		GetCurrentScene()->ForEachComponent<comp::Transform>([&](Entity& e, comp::Transform& transform)
			{
				std::string entityname = "Entity: " + std::to_string(static_cast<int>((entt::entity)e));
				
				ImGui::Separator();
				ImGui::Text(entityname.c_str());
				ImGui::DragFloat3(("Position##" + std::to_string(static_cast<int>((entt::entity)e))).c_str(), (float*)&transform.position);
				ImGui::DragFloat3(("Rotation##" + std::to_string(static_cast<int>((entt::entity)e))).c_str(), (float*)&transform.rotation, dx::XMConvertToRadians(1.f));

				if (ImGui::Button(("Remove##" + std::to_string(static_cast<int>((entt::entity)e))).c_str()))
				{
					e.Destroy();
				}
				ImGui::Spacing();
			});

		
	}
	
	if (ImGui::CollapsingHeader("Renderable"))
	{
		GetCurrentScene()->ForEachComponent<comp::Renderable>([&](Entity& e, comp::Renderable& renderable)
			{
				std::string entityname = "Entity: " + std::to_string(static_cast<int>((entt::entity)e));

				ImGui::Separator();
				ImGui::Text(entityname.c_str());
				ImGui::Text("Change 'mtl-file'");
				char str[30] = "";
				ImGui::InputText("New material", str, IM_ARRAYSIZE(str));
				if (ImGui::IsKeyPressedMap(ImGuiKey_Enter))
				{
					renderable.model->ChangeMaterial(str);
				}
				ImGui::Spacing();
			});
	}
	if (ImGui::CollapsingHeader("Light"))
	{

		GetCurrentScene()->ForEachComponent<comp::Light>([&](Entity& e, comp::Light& light)
			{
				std::string entityname = "Entity: " + std::to_string(static_cast<int>((entt::entity)e));
				
				ImGui::Separator();
				ImGui::Text(entityname.c_str());
				ImGui::SameLine();
				std::string index = std::to_string(light.index);
				ImGui::Text("Light index: %d", light.index);
				bool edited = false;
				if (ImGui::ColorEdit4(("Color##" + index).c_str(), (float*)&light.lightData.color)) 
					edited = true;
				
				if (ImGui::DragFloat3(("Direction##" + index).c_str(), (float*)&light.lightData.direction))
					edited = true;

				if (ImGui::Checkbox(("Enabled##" + index).c_str(), (bool*)&light.lightData.enabled))
					edited = true;

				if (ImGui::DragFloat3(("Position##" + index).c_str(), (float*)&light.lightData.position))
					edited = true;
				
				if (ImGui::InputFloat(("Range##" + index).c_str(), &light.lightData.range))
					edited = true;
				
				const char* const items[2] = { "Directional", "Point" };
				if (ImGui::ListBox(("Type##" + index).c_str(), (int*)&light.lightData.type, items, 2))
					edited = true;
				
				if (edited)
				{
					GetCurrentScene()->GetLights()->EditLight(light.lightData, light.index);
				}

				ImGui::Spacing();
			});

	}
	if (ImGui::CollapsingHeader("Network"))
	{

		GetCurrentScene()->ForEachComponent<comp::Network>([&](Entity& e, comp::Network& net)
			{
				std::string entityname = "Entity: " + std::to_string(static_cast<int>((entt::entity)e));
				std::string netID = "Network ID: " + std::to_string(net.id);

				ImGui::Separator();
				ImGui::Text(entityname.c_str());
				ImGui::Text(netID.c_str());

				ImGui::Spacing();
			});

	}
	if (ImGui::CollapsingHeader("Colliders"))
	{
		ImGui::Text("Bounding Oriented Box");
		ImGui::Spacing();

		GetCurrentScene()->ForEachComponent<comp::BoundingOrientedBox>([&](Entity& e, comp::BoundingOrientedBox& box)
			{
				std::string id = std::to_string(static_cast<int>((entt::entity)e));
				
				ImGui::Text("Entity: %d", (entt::entity)e);
				
				ImGui::DragFloat3(("Center##" + id).c_str(), (float*)&box.Center);
				ImGui::DragFloat3(("Extents##" + id).c_str(), (float*)&box.Extents);
				ImGui::DragFloat3(("Orientation##" + id).c_str(), (float*)&box.Orientation);

				ImGui::Spacing();
				ImGui::Separator();
			});

		ImGui::Text("Bounding Sphere");
		ImGui::Spacing();
		GetCurrentScene()->ForEachComponent<comp::BoundingSphere>([&](Entity& e, comp::BoundingSphere& s)
			{
				std::string id = std::to_string(static_cast<int>((entt::entity)e));

				ImGui::Text("Entity: %d", (entt::entity)e);

				ImGui::DragFloat3(("Center##" + id).c_str(), (float*)&s.Center);
				ImGui::DragFloat(("Radius##" + id).c_str(), (float*)&s.Radius, 0.1f);

				ImGui::Spacing();
				ImGui::Separator();
			});

	}

	ImGui::End();

	ImGui::Begin("Camera");
	{
		Camera* currentCam = GetCurrentScene()->GetCurrentCamera();
		if (currentCam)
		{
			const std::string position = "Position: " + std::to_string(currentCam->GetPosition().x)+ " " + std::to_string(currentCam->GetPosition().y) + " " + std::to_string(currentCam->GetPosition().z);
			ImGui::Separator();
			ImGui::Text(position.c_str());
			ImGui::DragFloat("Zoom: ", &currentCam->m_zoomValue, 0.01f, 0.0001f, 1.0f);
			ImGui::DragFloat("Near Plane : ", &currentCam->m_nearPlane, 0.1f , 0.0001f, currentCam->m_farPlane-1);
			ImGui::DragFloat("Far Plane: ", &currentCam->m_farPlane, 0.1f, currentCam->m_nearPlane+1);
			ImGui::DragFloat3("Position: ", (float*)&currentCam->m_position, 0.1f);
			ImGui::DragFloat3("Rotation: ", (float*)&currentCam->m_rollPitchYaw, 0.1f, 0.0f);
			ImGui::Spacing();
		}
		else
		{
			ImGui::Text("No Camera");
		}
	};
	ImGui::End();


	ImGui::Begin("Render Pass");
	{
		ImGui::Checkbox("Render Colliders", GetCurrentScene()->GetIsRenderingColliders());
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
			if (GetCurrentScene()->IsRenderReady() && GetCurrentScene()->IsRenderDebugReady() && rtd::Handler2D::Get().IsRenderReady())
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
	m_frameTime.update = dt;

	InputSystem::Get().UpdateEvents();
	rtd::Handler2D::Update();

	MSG msg = { nullptr };
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
		{
			Shutdown();
		}
	}
	// todo temp
	if (InputSystem::Get().CheckMouseKey(MouseKey::RIGHT, KeyState::PRESSED))
	{
		InputSystem::Get().SwitchMouseMode();

		LOG_INFO("Switched mouse Mode");
	}

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

	BasicEngine::Update(dt);

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

	{
		PROFILE_SCOPE("Render D3D11");
		/*
			Render 3D
		*/
		m_renderer.ClearFrame();
		m_renderer.Render(GetCurrentScene());
	}

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
		rtd::Handler2D::Get().Render();
		D2D1Core::Present();
	}

	

	{
		PROFILE_SCOPE("Present");
		D3D11Core::Get().SwapChain()->Present(0, 0);
	}


}