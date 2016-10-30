#pragma once

#include "Engine/EngineBase.h"

using namespace StenGine;

namespace SGGame
{

class Game : public EngineBase
{
public:
	Game();
	virtual void GameInit() override;
};

}