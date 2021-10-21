#pragma once

/*
	The knowledger and controller for different game states.
	States such as, In Menu, In Game, and more helps the game
	decide on what logic to include and run.
*/
class GameStateHandler
{
private:

	static GameState m_currentState;

public:

	auto& Get()
	{
		static GameStateHandler instance;
		return instance;
	}

};

/*
	The different states available.
*/
enum class GameState
{
	DEFAULT,
	MAIN_MENU,
	IN_GAME,
	IN_OPTIONS,

	NROF
};