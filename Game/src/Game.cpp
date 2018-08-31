#include "Scene/GameObjectManager.h"

#include "Game.h"

#include "GameObject/box.h"
#include "GameObject/sphere.h"
#include "GameObject/zombie.h"
#include "GameObject/Plane.h"
#include "GameObject/Plant.h"
#include "GameObject/House.h"
#include "GameObject/Dragon.h"
#include "GameObject/GameTerrain.h"
#include "GameObject/DebugCamera.h"
#include "GameObject/ThirdPersonCamera.h"

#define MIN_SCENE 1

std::unique_ptr<StenGine::EngineBase> CreateGame() 
{
	return std::move(std::make_unique<SGGame::Game>());
}

namespace SGGame
{

Game::Game()
{

}

void Game::GameInit()
{
	// TODO move these to somewhere else
	REGISTER_GAMEOBJECT(Box);
	REGISTER_GAMEOBJECT(Sphere);
	REGISTER_GAMEOBJECT(Zombie);
	REGISTER_GAMEOBJECT(Plane);
	REGISTER_GAMEOBJECT(Plant);
	REGISTER_GAMEOBJECT(House);
	REGISTER_GAMEOBJECT(Dragon);
	REGISTER_GAMEOBJECT(GameTerrain);
	REGISTER_GAMEOBJECT(DebugCamera);
	REGISTER_GAMEOBJECT(ThirdPersonCamera);
}

}