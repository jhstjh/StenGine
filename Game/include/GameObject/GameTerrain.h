#pragma once

#include "Scene/GameObject.h"

using namespace StenGine;

namespace SGGame
{

class GameTerrain : public GameObject
{
public:
	DECLARE_TYPE(GameTerrain)

	GameTerrain();
};

}