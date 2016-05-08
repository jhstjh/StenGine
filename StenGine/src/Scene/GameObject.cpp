#include "Scene/GameObject.h"
#include "Scene/Transform.h"
#include "imgui.h"

#include <algorithm>

namespace StenGine
{

GameObject::GameObject(const char* name, float tx, float ty, float tz, float rx, float ry, float rz, float sx, float sy, float sz) {
	XMMATRIX I = XMMatrixIdentity();
	m_name = std::string(name);

	m_transform = new Transform(tx, ty, tz, rx, ry, rz, sx, sy, sz); // will be cleanup in component
	m_components.push_back(m_transform);
}

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
	// HACK!!!!!!
	if (m_name == "sphere")
	{
		m_transform->RotateAroundY(-Timer::GetDeltaTime() * 3.14159f);
	}
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