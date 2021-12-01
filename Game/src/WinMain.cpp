
#include "Game.h"


// The main entry point of the game.
#ifdef _DEBUG
int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Game game;
	game.Start();

	return 0;
}
#else
#include <Windows.h>
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	Game game;
	game.Start();

	return 0;
}

#endif