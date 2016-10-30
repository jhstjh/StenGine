#pragma once

#include "System/SingletonClass.h"
#include "Scene/GameObject.h"
#include <vector>

namespace StenGine
{

class GameObjectManager : public SingletonClass<GameObjectManager>
{
public:
	GameObjectManager();
	~GameObjectManager();
	void LoadScene();
	void Update();

	void DrawMenu();
	void Add(GameObject* gameObject) { m_gameObjects.push_back(gameObject); }
private:
	std::vector<GameObject*> m_gameObjects;
};
}
