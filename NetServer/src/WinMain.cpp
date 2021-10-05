#include "NetServerPCH.h"
#include "ServerGame.h"

//------------------------------------------------------------------------------//
//																				//
//	Högerklicka på "NetServer"-projektet och välj: "Set as Startup Project".	//
//																				//
//------------------------------------------------------------------------------//
int main()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ServerGame game;
	game.Start();

	return 0;
}