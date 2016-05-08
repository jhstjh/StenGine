#include "Scene/Transform.h"
#include "imgui.h"
#include "Math/MathHelper.h"

#pragma warning(disable:4996)

namespace StenGine
{

const XMVECTOR VEC3ZERO = XMLoadFloat3(&XMFLOAT3(0, 0, 0));
const XMVECTOR QUATIDENT = XMQuaternionIdentity();
const XMVECTOR AXIS_X = XMLoadFloat3(&XMFLOAT3(1, 0, 0));
const XMVECTOR AXIS_Y = XMLoadFloat3(&XMFLOAT3(0, 1, 0));
const XMVECTOR AXIS_Z = XMLoadFloat3(&XMFLOAT3(0, 0, 1));

Transform::Transform(float tx, float ty, float tz, float rx, float ry, float rz, float sx, float sy, float sz)
{
	m_position = XMLoadFloat3(&XMFLOAT3(tx, ty, tz));
	m_rotation = XMQuaternionRotationRollPitchYaw(rx, ry, rz);
	m_scale = XMLoadFloat3(&XMFLOAT3(sx, sy, sz));
}

const XMFLOAT4X4* Transform::GetWorldTransform()
{
	XMMATRIX worldTrans = XMMatrixTransformation(VEC3ZERO, QUATIDENT, m_scale, VEC3ZERO, m_rotation, m_position);
	XMStoreFloat4x4(&m_worldTransform, worldTrans);
	return &m_worldTransform;
}

XMFLOAT3 Transform::GetPosition()
{
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, m_position);
	return pos;
}

void Transform::RotateAroundY(float radius)
{
	XMVECTOR rot = XMQuaternionRotationAxis(AXIS_Y, radius);
	m_rotation = XMQuaternionMultiply(m_rotation, rot);
}

void Transform::DrawMenu()
{
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
}

}

