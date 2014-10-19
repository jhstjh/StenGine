#ifndef __GAMEOBJECT__
#define __GAMEOBJECT__

#include "D3DIncludes.h"
#include "Component.h"

class Component;

class GameObject {
private:
	
	std::vector<Component*> m_components;

public:
	XMFLOAT4X4 m_worldTransform;
	GameObject();
	GameObject(float x, float y, float z);
	~GameObject();
	void SetPosition(float x, float y, float z);
	XMFLOAT3 GetPosition();
	void RotateAroundY(float radius);
	void AddComponent(Component* c);
	XMFLOAT4X4* GetWorldTransform() { return &m_worldTransform; }

	virtual void Update();
};


#endif // !__GAMEOBJECT__
