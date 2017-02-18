#pragma once

#include "Scene/GameObject.h"

using namespace StenGine;

namespace SGGame
{

class Plane : public GameObject
{
public:
	DECLARE_TYPE(Plane)

	Plane();
};

}