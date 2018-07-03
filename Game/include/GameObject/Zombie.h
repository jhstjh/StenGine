#pragma once

#include "Scene/GameObject.h"

using namespace StenGine;

namespace SGGame
{

class Zombie : public GameObject
{
public:
	Zombie();

	void Update() override;

private:

	void processIDLE();
	void processWALK();

	enum class State
	{
		IDLE,
		WALK,
	} mState { State::IDLE };
};

}