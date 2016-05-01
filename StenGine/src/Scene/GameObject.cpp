#include "Scene/GameObject.h"
#include "imgui.h"
#include "Math/MathHelper.h"

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

void GameObject::DrawMenu()
{
	ImGui::Text(m_name.c_str());

	if (ImGui::CollapsingHeader("Transform", nullptr, true, true))
	{
		ImGui::Text("Position");
		XMFLOAT3 pos = GetPosition();
		char scratch[16];
		sprintf(scratch, "%f", pos.x);
		ImGui::Text("X"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0)); ImGui::SameLine();
		sprintf(scratch, "%f", pos.y);
		ImGui::Text("Y"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0)); ImGui::SameLine();
		sprintf(scratch, "%f", pos.z);
		ImGui::Text("Z"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0));

		ImGui::Text("Rotation");
		sprintf(scratch, "%f", atan2(m_worldTransform._23, m_worldTransform._33) * 180 / PI);
		ImGui::Text("X"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0)); ImGui::SameLine();
		sprintf(scratch, "%f", atan2(-m_worldTransform._13, sqrt(m_worldTransform._23 * m_worldTransform._23 + m_worldTransform._33 * m_worldTransform._33)) * 180 / PI);
		ImGui::Text("Y"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0)); ImGui::SameLine();
		sprintf(scratch, "%f", atan2(m_worldTransform._12, m_worldTransform._11) * 180 / PI);
		ImGui::Text("Z"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0));

		ImGui::Text("Scale");
		ImGui::Text("X"); ImGui::SameLine(); ImGui::Button("1.0", ImVec2(80, 0)); ImGui::SameLine();
		ImGui::Text("Y"); ImGui::SameLine(); ImGui::Button("1.0", ImVec2(80, 0)); ImGui::SameLine();
		ImGui::Text("Z"); ImGui::SameLine(); ImGui::Button("1.0", ImVec2(80, 0));
	}

	for (auto& component : m_components)
	{
		component->DrawMenu();
	}
}

}