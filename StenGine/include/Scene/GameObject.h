#ifndef __GAMEOBJECT__
#define __GAMEOBJECT__

#include <Rpc.h>
#include "System/API/PlatformAPIDefs.h"
#include "Graphics/D3DIncludes.h"
#include "Scene/Component.h"
#include "Scene/Transform.h"

#define DECLARE_TYPE(Type) virtual const char* GetType() override { return #Type; }

namespace StenGine
{

class Component;
class GameObjectManager;

class GameObject {
public:
	virtual					~GameObject();
	void					SetName(const char* name) { mName = std::string(name); }
	std::string				GetName() const { return mName; }
	void					SetUUID(UUID uuid) { mUuid = uuid; }
	UUID					GetUUID() const { return mUuid; }
	void					AddComponent(Component* c);
	Component*				GetComponentByIdx(int index) { return mComponents[index]; }
	std::vector<Component*> GetAllComponents() { return mComponents; }

	template <class T>
	T* GetFirstComponentByType()
	{
		for (auto &comp : mComponents)
		{
			if (dynamic_cast<T*>comp)
				return comp;
		}
	}

	Transform* GetTransform() { return mTransform; };

	virtual void Update();
	virtual void DrawMenu();

	virtual const char* GetType() = 0;

	friend GameObjectManager;

protected:
	Transform*				mTransform;
	std::vector<Component*> mComponents;
	std::string				mName;
	UUID					mUuid;
};

}
#endif // !__GAMEOBJECT__
