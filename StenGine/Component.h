#ifndef __COMPONENT__
#define __COMPONENT__

#include "GameObject.h"
#include <vector>

class GameObject;

class Component {
public:
	GameObject* m_parent;
	std::vector<GameObject*> m_instancingParent;
	virtual ~Component();
};

#endif