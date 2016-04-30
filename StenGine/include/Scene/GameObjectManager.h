#pragma once

#include "System/SingletonClass.h"
#include "Scene/GameObject.h"
#include <vector>

namespace StenGine
{

class GameObjectManager : public SingletonClass<GameObjectManager>
{
public:
	~GameObjectManager();
	void LoadScene();
	void Update();

	void DrawMenu();

private:
	std::vector<GameObject*> m_gameObjects;
};
}
