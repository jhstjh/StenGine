#include "Scene/GameObject.h"
#include <algorithm>

namespace StenGine
{

GameObject::GameObject(const char* name) {
#if PLATFORM_WIN32
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_worldTransform, I);
#elif  PLATFORM_ANDROID
	m_worldTransform = ndk_helper::Mat4::Identity();
#endif
	m_name = std::string(name);
}

GameObject::GameObject(const char* name, float x, float y, float z) {
#if PLATFORM_WIN32
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_worldTransform, I);
#elif  PLATFORM_ANDROID
	m_worldTransform = ndk_helper::Mat4::Identity();
#endif
	m_name = std::string(name);
	SetPosition(x, y, z);
}

GameObject::~GameObject() {
	for (uint32_t i = 0; i < m_components.size(); i++) {
		m_components[i]->m_parents.erase(std::remove(m_components[i]->m_parents.begin(), m_components[i]->m_parents.end(), this), m_components[i]->m_parents.end());
		if (m_components[i]->m_parents.size() == 0) {
			//			SafeDelete(m_components[i]);
		}
	}
}

void GameObject::RotateAroundY(float radius) {
#if PLATFORM_WIN32
	XMStoreFloat4x4(&m_worldTransform, XMMatrixRotationY(radius) * XMLoadFloat4x4(&m_worldTransform));
#elif  PLATFORM_ANDROID
	ndk_helper::Mat4 rotMat;
	rotMat = rotMat.RotationY(radius);
	m_worldTransform = m_worldTransform * rotMat;
#endif
}

void GameObject::SetPosition(float x, float y, float z) {
#if PLATFORM_WIN32
	m_worldTransform._41 = x;
	m_worldTransform._42 = y;
	m_worldTransform._43 = z;
#elif  PLATFORM_ANDROID
	m_worldTransform = m_worldTransform.Translation(x, y, z);
#endif
}

XMFLOAT3 GameObject::GetPosition() {
#if PLATFORM_WIN32
	return XMFLOAT3(m_worldTransform._41, m_worldTransform._42, m_worldTransform._43);
#elif  PLATFORM_ANDROID
	//return XMFLOAT3(m_worldTransform[0][1], m_worldTransform._42, m_worldTransform._43);
#else
	return XMFLOAT3(0, 0, 0);
#endif
}

void GameObject::AddComponent(Component* c) {
	m_components.push_back(c);
	//c->m_parent = this;
	c->m_parents.push_back(this);
}

void GameObject::Update() {
#if PLATFORM_WIN32

	// HACK
	if (stricmp("sphere", m_name.c_str()) == 0)
		RotateAroundY(-Timer::GetDeltaTime() * 3.14159f);
#endif
}

}