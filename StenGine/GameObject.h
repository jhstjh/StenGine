#ifndef __GAMEOBJECT__
#define __GAMEOBJECT__

#if defined(PLATFORM_WIN32) || defined (SG_TOOL)
#include "D3DIncludes.h"
#elif defined PLATFORM_ANDROID
#include "AndroidType.h"
#endif
#include "Component.h"

class Component;

class GameObject {
protected:
	
	std::vector<Component*> m_components;
	std::string m_name;

public:
	XMFLOAT4X4 m_worldTransform;
	GameObject(const char* name);
	GameObject(const char* name, float x, float y, float z);
	~GameObject();
	void SetPosition(float x, float y, float z);
	XMFLOAT3 GetPosition();
	void RotateAroundY(float radius);
	void AddComponent(Component* c);
	XMFLOAT4X4* GetWorldTransform() { return &m_worldTransform; }
	Component* GetComponentByIdx(int index) { return m_components[index]; }

	virtual void Update();
};


#endif // !__GAMEOBJECT__
