#include "NetServerPCH.h"

#include "Window.h"
#include "Server.h"
void RedirectIoToConsole();

//------------------------------------------------------------------------------//
//																				//
//	Högerklicka på "NetServer"-projektet och välj: "Set as Startup Project".	//
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

	network::Server s;
	s.Start(4950);

	MSG msg = { nullptr };
	int i = 0;
	bool key = false;
	bool old_key = false;

	while (s.IsRunning())
	{
		// Service any and all pending Windows messages.
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			key = GetAsyncKeyState('1') & 0x8000;
		}
		if (key && !old_key)
		{
			s.Stop();
		}
		old_key = key;
	}

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