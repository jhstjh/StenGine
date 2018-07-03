#pragma once

#include <vector>
#include "Math/MathDefs.h"
#include "Graphics/D3DIncludes.h"
#include "Scene/Component.h"
#include "System/AlignedClass.h"

namespace StenGine
{

class Transform : public Component, public AlignedClass<16>
{
public:
	Transform(float tx = 0, float ty = 0, float tz = 0,
			  float rx = 0, float ry = 0, float rz = 0,
			  float sx = 1, float sy = 1, float sz = 1);

	const Mat4 &GetWorldTransform() const;
	const Mat4 &GetWorldTransformInversed() const;
	Vec3 GetPosition() const;
	void SetPosX(float x);
	void SetPosY(float y);
	void SetPosZ(float z);
	void RotateAroundY(float radius);
	void UpdateWorldTransform(Mat4& parent, bool parentDirty);
	void AddChild(Transform* child) { mChildren.push_back(child); }

	void MoveForward(float distance);
	void MoveBack(float distance) { MoveForward(-distance); }
	void MoveRight(float distance);
	void MoveLeft(float distance) { MoveRight(-distance); }
	
	void Rotate(float radians, const Vec3& axis, bool pre);

	virtual void DrawMenu();
	virtual void DrawMenuNodes(Transform*&, bool defaultOpen = false);

private:
	std::vector<Transform*> mChildren;

	Vec3 mPosition;
	Vec3 mRotationEuler;
	Quat mRotation;
	Vec3 mScale;

	Mat4 mWorldTransform;
	Mat4 mWorldTransformInversed;
	Mat4 mLocalTransform;
	bool mDirty;
};

}