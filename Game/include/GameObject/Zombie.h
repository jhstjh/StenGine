#pragma once

#include "Scene/GameObject.h"

using namespace StenGine;

namespace SGGame
{

class Zombie : public GameObject
{
public:
	DECLARE_TYPE(Zombie)

	Zombie();
};

}