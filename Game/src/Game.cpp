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

}

void Game::GameInit()
{
	GameObject::Instantiate<Box>("box0", 0.f, 1.2f, 0.f, 0.f, PI / 5);
	GameObject::Instantiate<Sphere>("sphere", 0.f, 3.7f, -0.5f);
	GameObject::Instantiate<Zombie>("Zombie", 7.f, -0.f, -0.5f, 0.f, PI, 0.f, 0.4f, 0.4f, 0.4f);

#if !MIN_SCENE || BUILD_RELEASE
	GameObject::Instantiate<Plane>("plane0", 4.f, 0.2f, 0.f);
	GameObject::Instantiate<Plane>("plane1", -4.f, 0.2f, 0.f);
	GameObject::Instantiate<Plant>("plants0", -4.f, 0.2f, 0.f);
	GameObject::Instantiate<House>("house0", 0.f, -0.1f, 20.f, 0.f, PI / 2);
	GameObject::Instantiate<Dragon>("dragon", 3.f, 0.2f, 0.f);
	GameObject::Instantiate<GameTerrain>("Terrain", 0, 0, -100);
#endif
}

}