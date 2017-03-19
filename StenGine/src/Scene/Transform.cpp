#include "Scene/GameObject.h"
#include "Scene/Transform.h"
#include "imgui.h"
#include "Math/MathHelper.h"

#pragma warning(disable:4996)

namespace StenGine
{

const Vec3 VEC3ZERO{ 0, 0, 0 };
const Quat QUATIDENT = Quat::identity;
const Vec3 AXIS_X{ 1, 0, 0 };
const Vec3 AXIS_Y{ 0, 1, 0 };
const Vec3 AXIS_Z{ 0, 0, 1 };

Transform::Transform(float tx, float ty, float tz, float rx, float ry, float rz, float sx, float sy, float sz)
{
	mPosition = { tx, ty, tz };
	mRotationEuler = { rx, ry, rz };
	mRotation = Quat::FromEulerAngles(mRotationEuler);
	mScale = { sx, sy, sz };
	mDirty = true;
}

const Mat4 &Transform::GetWorldTransform()
{
	return mWorldTransform;
}

Vec3 Transform::GetPosition() const
{
	return mPosition;
}

void Transform::RotateAroundY(float radius)
{
	Quat rot = Quat::FromAngleAxis(radius, AXIS_Y);
	mRotation = mRotation * rot;
	mRotationEuler = mRotation.ToEulerAngles();
	mDirty = true;
}

void Transform::UpdateWorldTransform(Mat4& parent, bool parentDirty)
{
	bool thisDirty = mDirty;
	if (mDirty)
	{
		mLocalTransform = Mat4::FromTranslationVector(mPosition) * mRotation.ToMatrix4() * Mat4::FromScaleVector(mScale);
		mDirty = false;
	}

	if (parentDirty || thisDirty)
	{
		mWorldTransform = parent * mLocalTransform;
	}

	for (auto &child : mChildren)
	{
		child->UpdateWorldTransform(mWorldTransform, mDirty || parentDirty);
	}
}

void Transform::DrawMenu()
{
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Vec3 mRotationEulerDegree = mRotationEuler / PI * 180.f;
		if (ImGui::InputFloat3("Position", &mPosition.x()))
		{
			mDirty |= true;
		}
		if (ImGui::InputFloat3("Rotation", &mRotationEulerDegree.x()))
		{
			mRotationEuler = mRotationEulerDegree * PI / 180.f;
			mRotation = Quat::FromEulerAngles(mRotationEuler);
			mDirty |= true;
		}
		if (ImGui::InputFloat3("Scale", &mScale.x()))
		{
			mDirty |= true;
		}
	}
}

void Transform::DrawMenuNodes(Transform* &selected, bool defaultOpen/* = false*/)
{
	uint32_t flag = ImGuiTreeNodeFlags_OpenOnArrow;
	if (mChildren.size() == 0)
	{
		flag |= ImGuiTreeNodeFlags_Leaf;
	}

	const char* nodeName = "";
	if (m_parents.size() == 0)
	{
		nodeName = "Root";
	}
	else
	{
		nodeName = m_parents[0]->GetName();
	}

	if (selected == this)
	{
		flag |= ImGuiTreeNodeFlags_Selected;
	}

	if (defaultOpen)
	{
		flag |= ImGuiTreeNodeFlags_DefaultOpen;
	}

	bool opened = ImGui::TreeNodeEx(nodeName, flag);
	if (ImGui::IsItemClicked())
	{
		selected = this;
	}

	if (opened)
	{
		for (auto child : mChildren)
		{
			child->DrawMenuNodes(selected);
		}
		ImGui::TreePop();
	}
}

}

