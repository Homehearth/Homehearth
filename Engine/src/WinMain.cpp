#include <EnginePCH.h>

#include "Engine.h"
#include "InputSystem.h"

#include <chrono>
#include <time.h>

void OnExit()
{
	T_DESTROY();
	resource::ResourceManager::Destroy();
	D2D1Core::Destroy();
}


// The main entry point of the engine.
int CALLBACK WinMain(	
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd )
{
	
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	MessageBoxA(nullptr, "Debug mode!", "Engine", 0);
#else
	MessageBoxA(nullptr, "Release mode!", "Engine", 0);
#endif
	
	std::atexit(OnExit);
	T_INIT(T_REC, thread::ThreadType::POOL_FIFO);
	resource::ResourceManager::Initialize();
	srand((unsigned int)time(NULL));
	
	Engine engine;
	engine.Setup(hInstance);
	
	// Create or get scene
	Scene& startScene = engine.GetScene("StartScene");
	// Set as current scene
	engine.SetScene(startScene);


	// Simple Ecs Demo
	// Simple Component
	struct SayHelloComponent {
		float timer;
		float messageTime;
		std::string message;
	};
	
	// Simple System to update components
	startScene.AddSystem([](entt::registry& reg, float dt)
	{
		auto view = reg.view<SayHelloComponent>();
		view.each([dt](const entt::entity& entity, SayHelloComponent& comp)
		{
			comp.timer += dt;
			if (comp.messageTime <= comp.timer) 
			{
				//LOG_CONSOLE("Entity ", entity, " says ", comp.message.c_str());
				std::cout << "Entity " << (int)entity << " says " << comp.message.c_str() << std::endl;
				comp.timer = 0.0f;
			}
		});
	});

	// Create test Entity
	entt::entity entity = startScene.CreateEntity();
	SayHelloComponent& comp = startScene.AddComponent<SayHelloComponent>(entity);
	comp.timer = 2.0f;
	comp.messageTime = 2.0f;
	comp.message = "Hello World!";

	// Create another test Entity
	entt::entity entity1 = startScene.CreateEntity();
	SayHelloComponent& comp1 = startScene.AddComponent<SayHelloComponent>(entity1);
	comp1.timer = 3.0f;
	comp1.messageTime = 3.0f;
	comp1.message = "Helo Wurld!";

	{
		auto lastTime = std::chrono::high_resolution_clock::now();

		//bool isRunning = true;
		MSG msg = { nullptr };
		while (engine.IsRunning())
		{
			// Service any and all pending Windows messages.
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT)
				{
					engine.Shutdown();
				}
			}
	
			// [InputSystem Test]
			InputEvent event;
			while (InputSystem::Get().PollEvent(event)) 
			{
				//LOG_CONSOLE("key_state: ", event.key_state, " key_code: ", event.key_code);
				if (event.key_code == VK_ESCAPE)
				{
					engine.Shutdown();
				}
			}
	
			auto now = std::chrono::high_resolution_clock::now();
			auto delta = std::chrono::duration_cast<std::chrono::duration<float>>(now - lastTime);
			lastTime = now;
			float dt = delta.count();
			engine.Update(dt);
			//engine.render();
		}
	}	
	engine.Shutdown();
	return 0;
}
	
