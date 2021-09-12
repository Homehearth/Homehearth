#include "NetServerPCH.h"

#include "Window.h"
#include "Server.h"
void RedirectIoToConsole();



//------------------------------------------------------------------------------//
//																				//
//	H�gerklicka p� "NetServer"-projektet och v�lj: "Set as Startup Project".	//
//																				//
//------------------------------------------------------------------------------//




// The main entry point of the engine.
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	MessageBoxA(nullptr, "Debug mode!", "NetServer", 0);
#else
	MessageBoxA(nullptr, "Release mode!", "NetServer", 0);
#endif

	RedirectIoToConsole();

	Window window;
	window.Initialize();
	T_INIT(T_REC, thread::ThreadType::POOL_FIFO);

	Server::Start(4950);

	bool isRunning = true;
	
	MSG msg = { nullptr };
	int i = 0;
	while (Server::IsRunning())
	{
		// Service any and all pending Windows messages.
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			isRunning = (msg.message != WM_QUIT);
		}

		InputEvent event;
		while (InputSystem::Get().PollEvent(event))
		{
			//LOG_CONSOLE("key_state: ", event.key_state, " key_code: ", event.key_code);
			if (event.key_code == VK_ESCAPE)
			{
				isRunning = false;
			}
		}
	}

	T_DESTROY();
	Server::Destroy();

	return 0;
}

void RedirectIoToConsole()
{
	AllocConsole();
	HANDLE stdHandle;
	int hConsole;
	FILE* fp;
	stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	hConsole = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
	fp = _fdopen(hConsole, "w");
	freopen_s(&fp, "CONOUT$", "w", stdout);
}