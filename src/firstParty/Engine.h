#pragma once

#include "ActorTemplateDB.h"
#include "glm/glm.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "SceneDB.h"

enum GAMESTATE { INTRO, MAIN, END };

class Engine
{
private:
	inline static std::string playerInput = "";

	inline static bool gameRunning = true;
	inline static bool won = false;
	inline static bool switchingScene = false;
	inline static bool newSceneSwitching = false;

	inline static int nextHealthLossFrame = 0;

	// Change this: inserting using string is SLOW, make string uint32_t instead, got lazy and used pointer instead
	// inline static std::unordered_set<const Actor*> actorScored;


	// Get input from the player
	static void GetInput();

	// Update all actors and gamestate
	static void Update();

public:

	// Loop the game until finished
	static void Run();

	inline static GAMESTATE gameState = INTRO;

};
