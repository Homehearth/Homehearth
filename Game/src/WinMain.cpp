
#include "Game.h"
#include "Client.h"


// The main entry point of the game.
int main()
{
	network::Client m_client;
	m_client.Connect("127.0.0.1", 4950);
	Game game;
	game.Run();
	return 0;
}
