#include <EnginePCH.h>

#include "Engine.h"
#include "InputSystem.h"

#include <chrono>


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
	comp.message = "Marco!";

	// Create another test Entity
	entt::entity entity1 = startScene.CreateEntity();
	SayHelloComponent& comp1 = startScene.AddComponent<SayHelloComponent>(entity1);
	comp1.timer = 3.0f;
	comp1.messageTime = 3.0f;
	comp1.message = "Polo!";


	engine.Start();

	return 0;
}
	
