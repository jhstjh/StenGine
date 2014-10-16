#include "GameObject.h"


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
		delete m_components[i];
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
	c->m_parent = this;
}
