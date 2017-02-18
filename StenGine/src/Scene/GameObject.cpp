#include "Scene/GameObject.h"
#include "Scene/Transform.h"
#include "imgui.h"

#include <algorithm>

namespace StenGine
{

GameObject::~GameObject() {
	for (uint32_t i = 0; i < mComponents.size(); i++) {
		mComponents[i]->m_parents.erase(std::remove(mComponents[i]->m_parents.begin(), mComponents[i]->m_parents.end(), this), mComponents[i]->m_parents.end());
		if (mComponents[i]->m_parents.size() == 0) {
			//			SafeDelete(m_components[i]);
		}
	}
}

void GameObject::AddComponent(Component* c) {
	mComponents.push_back(c);
	c->m_parents.push_back(this);
}

void GameObject::Update()
{

}

void GameObject::DrawMenu()
{
	ImGui::Text(mName.c_str());

	for (auto& component : mComponents)
	{
		component->DrawMenu();
	}
}

}