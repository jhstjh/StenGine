#ifndef __COMPONENT__
#define __COMPONENT__

#include <vector>

namespace StenGine
{

class GameObject;

class Component {
public:
	GameObject* mParent{ nullptr };
	virtual ~Component() = default;

	virtual void DrawMenu() = 0;
};

}
#endif