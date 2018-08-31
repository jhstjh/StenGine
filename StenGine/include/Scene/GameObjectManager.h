#pragma once

#include "System/SingletonClass.h"
#include "Scene/GameObject.h"
#include "Scene/GameObjectRegistry.h"
#include "Scene/Transform.h"

#define REGISTER_GAMEOBJECT(n) StenGine::GameObjectManager::Instance()->GetRegistry().RegisterGameObjectClass<n>(#n)

// TODO move these UUID function to somewhere else
namespace std {
template<> struct hash<UUID>
{
	size_t operator()(const UUID& uuid) const noexcept
	{
		RPC_STATUS status;
		auto ret = UuidHash(const_cast<UUID*>(&uuid), &status);
		return ret;
	}
};
}

inline bool operator < (const UUID &uuid1, const UUID &uuid2)
{
	if (uuid1.Data1 != uuid2.Data1)
	{
		return uuid1.Data1 < uuid2.Data1;
	}
	if (uuid1.Data2 != uuid2.Data2)
	{
		return uuid1.Data2 < uuid2.Data2;
	}
	if (uuid1.Data3 != uuid2.Data3)
	{
		return uuid1.Data3 < uuid2.Data3;
	}
	for (int i = 0; i < 8; i++)
	{
		if (uuid1.Data4[i] != uuid2.Data4[i])
		{
			return uuid1.Data4[i] < uuid2.Data4[i];
		}
	}
	return false;
}

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
	GameObjectRegistry &GetRegistry() { return mRegistry; }
	void BuildSceneHierarchy();

	GameObject* FindGameObjectByName(const std::string& name) const;

	GameObject* Instantiate(const char* objectType, UUID uuid, const char* name, UUID parent,
		float tx = 0, float ty = 0, float tz = 0,
		float rx = 0, float ry = 0, float rz = 0,
		float sx = 1, float sy = 1, float sz = 1)
	{
		auto gameObject = mRegistry.Instantiate(objectType);
		gameObject->SetName(name);
		auto transform = std::make_unique<Transform>(tx, ty, tz, rx, ry, rz, sx, sy, sz); // will be cleanup in component
		gameObject->AddComponent(std::move(transform));
		gameObject->m_transform = gameObject->GetFirstComponentByType<Transform>();
		gameObject->m_uuid = uuid;
		gameObject->m_parentUUID = parent;

		Add(uuid, gameObject);
		return gameObject;
	}

private:
	void Add(UUID uuid, GameObject* gameObject) { mGameObjects[uuid] = gameObject; }

	std::unordered_map<UUID, GameObject*> mGameObjects;
	GameObjectRegistry		 mRegistry;
	Transform				 mRoot;
};
}
