#pragma once

#include "System/SingletonClass.h"
#include "Scene/GameObject.h"
#include "Scene/GameObjectRegistry.h"
#include "Scene/Transform.h"
#include <vector>

#define REGISTER_GAMEOBJECT(n) StenGine::GameObjectManager::Instance()->GetRegistry().RegisterGameObjectClass<n>(#n)

namespace StenGine
{

class GameObjectManager : public SingletonClass<GameObjectManager>
{
public:
	GameObjectManager();
	~GameObjectManager();
	void LoadScene();
	void Update();
	void UpdateTransform();

	void DrawMenu();
	void Add(GameObject* gameObject) { mGameObjects.push_back(gameObject); }
	GameObjectRegistry &GetRegistry() { return mRegistry; }

	GameObject* Instantiate(const char* objectType, UUID uuid, const char* name, Transform* parent = nullptr,
		float tx = 0, float ty = 0, float tz = 0,
		float rx = 0, float ry = 0, float rz = 0,
		float sx = 1, float sy = 1, float sz = 1)
	{
		auto gameObject = mRegistry.Instantiate(objectType);
		gameObject->SetName(name);
		auto transform = new Transform(tx, ty, tz, rx, ry, rz, sx, sy, sz); // will be cleanup in component

		if (parent)
		{
			parent->AddChild(transform);
		}
		else
		{
			mRoot.AddChild(transform);
		}

		gameObject->AddComponent(transform);
		gameObject->m_transform = transform;

		RPC_STATUS status;
		if (UuidIsNil(&uuid, &status))
		{
			UuidCreate(&uuid);
		}
		gameObject->m_uuid = uuid;

		Add(gameObject);
		return gameObject;
	}

private:
	std::vector<GameObject*> mGameObjects;
	GameObjectRegistry		 mRegistry;
	Transform				 mRoot;
};
}
