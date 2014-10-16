#ifndef __COMPONENT__
#define __COMPONENT__

#include "GameObject.h"

class GameObject;

class Component {
public:
	GameObject* m_parent;
	virtual ~Component();
};

#endif