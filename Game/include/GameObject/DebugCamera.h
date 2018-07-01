#pragma once

#include "Scene/GameObject.h"

using namespace StenGine;

namespace SGGame
{

	class DebugCamera : public GameObject
	{
	public:
		DebugCamera();

		virtual void Update() override;
	};

}