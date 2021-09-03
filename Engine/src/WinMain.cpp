#include <EnginePCH.h>
#include <Window.h>
void OnExit()
{
	T_DESTROY();
	resource::ResourceManager::Destroy();
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	MessageBoxA(nullptr, "Hi and welcome to debug mode!", "Engine", 0);
#else
	MessageBoxA(nullptr, "Hi and welcome to release mode!", "Engine", 0);
#endif
	std::atexit(OnExit);
	T_INIT(T_REC, thread::ThreadType::POOL_FIFO);
	
	// Create main window.
	Window window;

	// Setup window configurations.
	Window::Desc config;
	config.hInstance = hInstance; config.nShowCmd = nCmdShow;
	config.width = 1920; config.height = 1080;
	config.title = L"Default window";
	if (!window.initialize(config))
	{
		LOG_ERROR("Could not initialize window.");
	}
	MSG msg = { nullptr };
	bool isRunning = true;
	//-- GAME LOOP --
	while (isRunning)
	{
		// Service any and all pending Windows messages.
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			isRunning = (msg.message != WM_QUIT);
		}
	}

	return 0;
}
	
