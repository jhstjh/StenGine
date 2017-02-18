#pragma once

#include "Scene/GameObject.h"

using namespace StenGine;

namespace SGGame
{

class Sphere : public GameObject
{
public:
	DECLARE_TYPE(Sphere)

	Sphere();

	virtual void Update() override;
};

}