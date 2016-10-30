#include "Scene/GameObject.h"
#include "Scene/Transform.h"
#include "imgui.h"

#include <algorithm>

namespace StenGine
{

GameObject::~GameObject() {
	for (uint32_t i = 0; i < m_components.size(); i++) {
		m_components[i]->m_parents.erase(std::remove(m_components[i]->m_parents.begin(), m_components[i]->m_parents.end(), this), m_components[i]->m_parents.end());
		if (m_components[i]->m_parents.size() == 0) {
			//			SafeDelete(m_components[i]);
		}
	}
}

void GameObject::AddComponent(Component* c) {
	m_components.push_back(c);
	//c->m_parent = this;
	c->m_parents.push_back(this);
}

void GameObject::Update()
{

}

void GameObject::DrawMenu()
{
	ImGui::Text(m_name.c_str());

	for (auto& component : m_components)
	{
		component->DrawMenu();
	}
}

}