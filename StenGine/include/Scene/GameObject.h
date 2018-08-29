#ifndef __GAMEOBJECT__
#define __GAMEOBJECT__

#include <memory>
#include <Rpc.h>
#include "System/API/PlatformAPIDefs.h"
#include "Graphics/D3DIncludes.h"
#include "Scene/Component.h"
#include "Scene/Transform.h"

namespace StenGine
{

class GameObjectManager;
using UniqueComponent = std::unique_ptr<Component>;

class GameObject {
public:
	virtual ~GameObject();
	void SetName(const char* name) { m_name = std::string(name); }
	const char* GetName() { return m_name.c_str(); }
	void SetUUID(UUID uuid) { m_uuid = uuid; }
	void SetEnabled(bool enabled) { m_enabled = enabled; }
	void AddComponent(UniqueComponent c);
	Component* GetComponentByIdx(int index) { return m_components[index].get(); }

	template <class T>
	T* GetFirstComponentByType()
	{
		for (auto &&comp : m_components)
		{
			auto ret = dynamic_cast<T*>(comp.get());
			if (ret)
				return ret;
		}
		return nullptr;
	}

	Transform* GetTransform() { return m_transform; };

	virtual void Start() {};
	virtual void Update() {};
	virtual void DrawMenu();

	friend GameObjectManager;

protected:
	Transform* m_transform {nullptr};
	std::vector<UniqueComponent> m_components;
	std::string m_name;
	UUID m_uuid;
	UUID m_parentUUID;
	bool m_started{ false };
	bool m_enabled{ true };
};

}
#endif // !__GAMEOBJECT__
