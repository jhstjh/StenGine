#pragma once
#include <functional>
#include <unordered_map>
#include "Scene/GameObject.h"

namespace StenGine
{

class GameObjectRegistry
{
public:
	template <typename T>
	void RegisterGameObjectClass(const char* name)
	{
		assert(mGameObjectMap.find(name) == mGameObjectMap.end());
		mGameObjectMap[name] = []() -> GameObject* { return static_cast<GameObject*>(new T()); };
	}

	GameObject* Instantiate(const char* name)
	{
		auto find = mGameObjectMap.find(name);
		if (find != mGameObjectMap.end())
		{
			auto ret = find->second();
			assert(ret);
			return ret;
		}
		assert(false); // Forget to register class?
		return nullptr;
	}

private:
	using GameObjectCreator = std::function<GameObject*()>;
	std::unordered_map<const char*, GameObjectCreator> mGameObjectMap;
};

}