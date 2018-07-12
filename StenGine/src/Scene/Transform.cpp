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
	mLocalTransform = Mat4::FromTranslationVector(mPosition) * mRotation.ToMatrix4() * Mat4::FromScaleVector(mScale);
	mDirty = true;
}

const Mat4 &Transform::GetWorldTransform() const
{
	return mWorldTransform;
}

const Mat4 &Transform::GetPrevWorldTransform() const
{
	return mPrevWorldTransform;
}

const Mat4 &Transform::GetWorldTransformInversed() const
{
	return mWorldTransformInversed;
}

Vec3 Transform::GetPosition() const
{
	return mPosition;
}

void Transform::SetPosX(float x)
{
	mPosition.x() = x;
	mDirty = true;
}

void Transform::SetPosY(float y)
{
	mPosition.y() = y;
	mDirty = true;
}

void Transform::SetPosZ(float z)
{
	mPosition.z() = z;
	mDirty = true;
}

void Transform::RotateAroundY(float radius)
{
	Quat rot = Quat::FromAngleAxis(radius, AXIS_Y);
	mRotation = mRotation * rot;
	mRotationEuler = mRotation.ToEulerAngles();
	mDirty = true;
}

void Transform::LookAt(const Vec3 &target, const Vec3 &worldUp)
{
	Vec3 forward = target - mPosition;
	forward.Normalize();
	Vec3 right = Vec3::CrossProduct(worldUp, forward).Normalized();
	Vec3 realUp = Vec3::CrossProduct(forward, right).Normalized();
	
	Mat3 rot = Mat3(right, realUp, forward);
	mRotation = Quat::FromMatrix(rot);
	mRotationEuler = mRotation.ToEulerAngles();
	mDirty = true;
}

void Transform::UpdateWorldTransform(Mat4& parent, bool parentDirty)
{
	mPrevWorldTransform = mWorldTransform;

	bool thisDirty = mDirty;
	if (mDirty)
	{
		mLocalTransform = Mat4::FromTranslationVector(mPosition) * mRotation.ToMatrix4() * Mat4::FromScaleVector(mScale);
		mDirty = false;
	}

	if (parentDirty || thisDirty)
	{
		mWorldTransform = parent * mLocalTransform;
		mWorldTransformInversed = mWorldTransform.Inverse();
	}

	for (auto &child : mChildren)
	{
		child->UpdateWorldTransform(mWorldTransform, thisDirty || parentDirty);
	}
}

void Transform::MoveForward(float distance)
{
	Vec3 offset = MatrixHelper::GetForward(mWorldTransform);
	mPosition.x() += offset[0] * distance;
	mPosition.y() += offset[1] * distance;
	mPosition.z() += offset[2] * distance;

	mDirty = true;
}

void Transform::MoveRight(float distance)
{
	Vec3 offset = MatrixHelper::GetRight(mWorldTransform);
	mPosition.x() += offset[0] * distance * mScale.x();
	mPosition.y() += offset[1] * distance * mScale.x();
	mPosition.z() += offset[2] * distance * mScale.x();

	mDirty = true;
}

void Transform::Rotate(float radians, const Vec3& axis, bool pre)
{
	auto rot = Quat::FromAngleAxis(radians, axis);
	if (pre)
	{
		mRotation = mRotation * rot;
	}
	else
	{
		mRotation = rot * mRotation;
	}
	mRotationEuler = mRotation.ToEulerAngles();
	mDirty = true;
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
	if (mParent == nullptr)
	{
		nodeName = "Root";
	}
	else
	{
		nodeName = mParent->GetName();
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

