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
	: mTx(tx)
	, mTy(ty)
	, mTz(tz)
	, mRx(rx)
	, mRy(ry)
	, mRz(rz)
	, mSx(sx)
	, mSy(sy)
	, mSz(sz)
{
	mPosition = XMLoadFloat3(&XMFLOAT3(tx, ty, tz));
	mRotation = XMQuaternionRotationRollPitchYaw(rx, ry, rz);
	mScale = XMLoadFloat3(&XMFLOAT3(sx, sy, sz));
}

const XMFLOAT4X4* Transform::GetWorldTransform()
{
	XMMATRIX worldTrans = XMMatrixTransformation(VEC3ZERO, QUATIDENT, mScale, VEC3ZERO, mRotation, mPosition);
	XMStoreFloat4x4(&mWorldTransform, worldTrans);
	return &mWorldTransform;
}

XMFLOAT3 Transform::GetPosition() const
{
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, mPosition);
	return pos;
}

void Transform::RotateAroundY(float radius)
{
	XMVECTOR rot = XMQuaternionRotationAxis(AXIS_Y, radius);
	mRotation = XMQuaternionMultiply(mRotation, rot);
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
		sprintf(scratch, "%f", atan2(mWorldTransform._23, mWorldTransform._33) * 180 / PI);
		ImGui::Text("X"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0)); ImGui::SameLine();
		sprintf(scratch, "%f", atan2(-mWorldTransform._13, sqrt(mWorldTransform._23 * mWorldTransform._23 + mWorldTransform._33 * mWorldTransform._33)) * 180 / PI);
		ImGui::Text("Y"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0)); ImGui::SameLine();
		sprintf(scratch, "%f", atan2(mWorldTransform._12, mWorldTransform._11) * 180 / PI);
		ImGui::Text("Z"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0));

		ImGui::Text("Scale");
		ImGui::Text("X"); ImGui::SameLine(); ImGui::Button("1.0", ImVec2(80, 0)); ImGui::SameLine();
		ImGui::Text("Y"); ImGui::SameLine(); ImGui::Button("1.0", ImVec2(80, 0)); ImGui::SameLine();
		ImGui::Text("Z"); ImGui::SameLine(); ImGui::Button("1.0", ImVec2(80, 0));
	}
}

tinyxml2::XMLElement* Transform::Save(tinyxml2::XMLDocument &doc) const
{
	tinyxml2::XMLElement* pTransfrom = doc.NewElement("Transform");

	tinyxml2::XMLElement* pPosition = doc.NewElement("Position");
	pPosition->SetAttribute("x", mTx);
	pPosition->SetAttribute("y", mTy);
	pPosition->SetAttribute("z", mTz);
	pTransfrom->InsertEndChild(pPosition);

	tinyxml2::XMLElement* pRotation = doc.NewElement("Rotation");
	pRotation->SetAttribute("x", mRx); 
	pRotation->SetAttribute("y", mRy); 
	pRotation->SetAttribute("z", mRz); 
	pTransfrom->InsertEndChild(pRotation);

	tinyxml2::XMLElement* pScale = doc.NewElement("Scale");
	pScale->SetAttribute("x", mSx);
	pScale->SetAttribute("y", mSy);
	pScale->SetAttribute("z", mSz);
	pTransfrom->InsertEndChild(pScale);

	return pTransfrom;
}

}

