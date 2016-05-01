#ifndef __COMPONENT__
#define __COMPONENT__

#include "GameObject.h"
#include <vector>

namespace StenGine
{

class GameObject;

class Component {
public:
	GameObject* m_parent;
	std::vector<GameObject*> m_parents;
	virtual ~Component();

	virtual void DrawMenu() = 0;
};

}
#endif