#ifndef __GAMEOBJECT__
#define __GAMEOBJECT__

#include <Rpc.h>
#include "System/API/PlatformAPIDefs.h"
#include "Graphics/D3DIncludes.h"
#include "Scene/Component.h"
#include "Scene/Transform.h"

namespace StenGine
{

class Component;
class GameObjectManager;

class GameObject {
public:
	virtual ~GameObject();
	void SetName(const char* name) { m_name = std::string(name); }
	void SetUUID(UUID uuid) { m_uuid = uuid; }
	void AddComponent(Component* c);
	Component* GetComponentByIdx(int index) { return m_components[index]; }

	template <class T>
	T* GetFirstComponentByType()
	{
		for (auto &comp : m_components)
		{
			if (dynamic_cast<T*>comp)
				return comp;
		}
	}

	Transform* GetTransform() { return m_transform; };

	virtual void Update();
	virtual void DrawMenu();

	friend GameObjectManager;

protected:
	Transform* m_transform;
	std::vector<Component*> m_components;
	std::string m_name;
	UUID m_uuid;
};

}
#endif // !__GAMEOBJECT__
