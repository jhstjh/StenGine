#pragma once

#include "Scene/GameObject.h"

using namespace StenGine;

namespace SGGame
{

class Sphere : public GameObject
{
public:
	Sphere();

	virtual void Update() override;
};

}