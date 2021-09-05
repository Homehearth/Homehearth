#include <EnginePCH.h>

#include "Engine.h"

void OnExit()
{
	T_DESTROY();
	resource::ResourceManager::Destroy();
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
	
	Engine engine;
	engine.setup(hInstance);
	{
		bool isRunning = true;
		MSG msg = { nullptr };
		while (isRunning)
		{
			// Service any and all pending Windows messages.
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				isRunning = (msg.message != WM_QUIT);
			}
	
			// [InputSystem test]
			InputEvent event;
			while (InputSystem::Get().pollEvent(event)) {
				std::cout << "key_state: " << event.key_state << " key_code: " << event.key_code << std::endl;
				if (event.key_code == VK_ESCAPE)
					isRunning = false;
			}
	
			const float dt = 0.16f;
			engine.update(dt);
			//engine.render();
		}
	}	
	engine.shutdown();
	return 0;
}
	
