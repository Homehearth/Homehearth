#include "NetServerPCH.h"
#include "ServerGame.h"

//------------------------------------------------------------------------------//
//																				//
//	H�gerklicka p� "NetServer"-projektet och v�lj: "Set as Startup Project".	//
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