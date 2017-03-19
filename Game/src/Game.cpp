#include "Game.h"
#include <memory>

#include "GameObject/box.h"
#include "GameObject/sphere.h"
#include "GameObject/zombie.h"
#include "GameObject/Plane.h"
#include "GameObject/Plant.h"
#include "GameObject/House.h"
#include "GameObject/Dragon.h"
#include "GameObject/GameTerrain.h"

#define MIN_SCENE 1

std::unique_ptr<StenGine::EngineBase> CreateGame() 
{
	return std::move(std::make_unique<SGGame::Game>());
}

namespace SGGame
{

Game::Game()
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
}

void Game::GameInit()
{
	static UUID NIL_UUID;
	UuidCreateNil(&NIL_UUID);

	auto box = GameObjectManager::Instance()->Instantiate("Box", NIL_UUID, "box0", nullptr, 0.f, 1.2f, 0.f, 0.f, PI / 5);
	GameObjectManager::Instance()->Instantiate("Sphere", NIL_UUID, "sphere", box->GetTransform(), 0.f, 2.5f, -0.5f);
	GameObjectManager::Instance()->Instantiate("Zombie", NIL_UUID, "Zombie", nullptr, 7.f, -0.f, -0.5f, 0.f, PI, 0.f, 0.4f, 0.4f, 0.4f);

#if !MIN_SCENE || BUILD_RELEASE
	GameObjectManager::Instance()->Instantiate("Plane", NIL_UUID,"plane0", nullptr, 4.f, 0.2f, 0.f);
	GameObjectManager::Instance()->Instantiate("Plane", NIL_UUID,"plane1", nullptr, -4.f, 0.2f, 0.f);
	GameObjectManager::Instance()->Instantiate("Plant", NIL_UUID,"plants0", nullptr, -4.f, 0.2f, 0.f);
	GameObjectManager::Instance()->Instantiate("House", NIL_UUID,"house0", nullptr, 0.f, -0.1f, 20.f, 0.f, PI / 2);
	GameObjectManager::Instance()->Instantiate("Dragon", NIL_UUID, "dragon", nullptr, 3.f, 0.2f, 0.f);
	GameObjectManager::Instance()->Instantiate("GameTerrain", NIL_UUID, "Terrain", nullptr, 0, 0, -100);
#endif
}

}