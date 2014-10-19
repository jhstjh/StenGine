#include "GameObject.h"
#include <algorithm>

GameObject::GameObject() {
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_worldTransform, I);
}

GameObject::GameObject(float x, float y, float z) {
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_worldTransform, I);

	SetPosition(x, y, z);
}

GameObject::~GameObject() {
	for (int i = 0; i < m_components.size(); i++) {
		m_components[i]->m_parents.erase(std::remove(m_components[i]->m_parents.begin(), m_components[i]->m_parents.end(), this), m_components[i]->m_parents.end());
		if (m_components[i]->m_parents.size() == 0) {
//			SafeDelete(m_components[i]);
		}
	}
}

void GameObject::RotateAroundY(float radius) {
	XMMATRIX R = XMMatrixRotationY(radius);
	XMStoreFloat4x4(&m_worldTransform, XMLoadFloat4x4(&m_worldTransform) * R);
}

void GameObject::SetPosition(float x, float y, float z) {
	m_worldTransform._41 = x;
	m_worldTransform._42 = y;
	m_worldTransform._43 = z;
}

void GameObject::AddComponent(Component* c) {
	m_components.push_back(c); 
	//c->m_parent = this;
	c->m_parents.push_back(this);
}

void GameObject::Update() {
	RotateAroundY(Timer::GetDeltaTime() * 3.14159);
}