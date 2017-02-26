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
	mRotation = Quat::FromEulerAngles({ rx, ry, rz });
	mScale = { sx, sy, sz };
}

const Mat4 &Transform::GetWorldTransform()
{
	mWorldTransform = Mat4::FromTranslationVector(mPosition) * mRotation.ToMatrix4() * Mat4::FromScaleVector(mScale);
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
}

void Transform::DrawMenu()
{
	if (ImGui::CollapsingHeader("Transform", nullptr, true, true))
	{
		ImGui::Text("Position");
		Vec3 pos = GetPosition();
		char scratch[16];
		sprintf(scratch, "%f", pos.x());
		ImGui::Text("X"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0)); ImGui::SameLine();
		sprintf(scratch, "%f", pos.y());
		ImGui::Text("Y"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0)); ImGui::SameLine();
		sprintf(scratch, "%f", pos.z());
		ImGui::Text("Z"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0));

		ImGui::Text("Rotation");
		sprintf(scratch, "%f", atan2(mWorldTransform(2, 1), mWorldTransform(2, 2)) * 180 / PI);
		ImGui::Text("X"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0)); ImGui::SameLine();
		sprintf(scratch, "%f", atan2(-mWorldTransform(2, 0), sqrt(mWorldTransform(2, 1) * mWorldTransform(2, 1) + mWorldTransform(2, 2) * mWorldTransform(2, 2))) * 180 / PI);
		ImGui::Text("Y"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0)); ImGui::SameLine();
		sprintf(scratch, "%f", atan2(mWorldTransform(1, 0), mWorldTransform(0, 0)) * 180 / PI);
		ImGui::Text("Z"); ImGui::SameLine(); ImGui::Button(scratch, ImVec2(80, 0));

		ImGui::Text("Scale");
		ImGui::Text("X"); ImGui::SameLine(); ImGui::Button("1.0", ImVec2(80, 0)); ImGui::SameLine();
		ImGui::Text("Y"); ImGui::SameLine(); ImGui::Button("1.0", ImVec2(80, 0)); ImGui::SameLine();
		ImGui::Text("Z"); ImGui::SameLine(); ImGui::Button("1.0", ImVec2(80, 0));
	}
}

}

