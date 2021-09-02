#include <EnginePCH.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	MessageBox(nullptr, L"Hi and welcome to debug mode!", L"Engine", 0);
#else
	MessageBox(nullptr, L"Hi and welcome to release mode!", L"Engine", 0);
#endif
	




	
	return 0;
}