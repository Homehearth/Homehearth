#include <EnginePCH.h>

void OnExit()
{
	T_DESTROY();
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
	
	LOG_INFO("Engine has successfully started.");
	LOG_WARNING("Engine is about the crash.");
	LOG_ERROR("Engine has successfully crashed.");
	return 0;
}
