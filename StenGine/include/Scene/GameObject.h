#ifndef __GAMEOBJECT__
#define __GAMEOBJECT__

#include "System/API/PlatformAPIDefs.h"
#include "Graphics/D3DIncludes.h"
#include "Scene/Component.h"
#include "Scene/Transform.h"

namespace StenGine
{

class Component;
class GameObjectManager;

class GameObject {
protected:
	Transform* m_transform;
	std::vector<Component*> m_components;
	std::string m_name;

public:
	GameObject(const char* name, 
			   float tx = 0, float ty = 0, float tz = 0,
			   float rx = 0, float ry = 0, float rz = 0,
			   float sx = 1, float sy = 1, float sz = 1);
	~GameObject();
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
};

}
#endif // !__GAMEOBJECT__
