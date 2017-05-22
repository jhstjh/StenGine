#include "Scene/GameObject.h"
#include "Scene/Transform.h"
#include "imgui.h"

#include <algorithm>

namespace StenGine
{

GameObject::~GameObject() 
{

}

void GameObject::AddComponent(UniqueComponent c) {
	c->mParent = this;
	m_components.push_back(std::move(c));
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