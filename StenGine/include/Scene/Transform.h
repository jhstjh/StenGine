#pragma once

#include "Math/MathDefs.h"
#include "Graphics/D3DIncludes.h"
#include "Scene/Component.h"
#include "System/AlignedClass.h"

namespace StenGine
{

class Component;

class Transform : public Component, public AlignedClass<16>
{
public:
	Transform(float tx = 0, float ty = 0, float tz = 0,
			  float rx = 0, float ry = 0, float rz = 0,
			  float sx = 1, float sy = 1, float sz = 1);

	const Mat4 &GetWorldTransform();
	Vec3 GetPosition() const;
	void RotateAroundY(float radius);

	virtual void DrawMenu();

private:
	Transform* mParent;

	Vec3 mPosition;
	Quat mRotation;
	Vec3 mScale;

	Mat4 mWorldTransform;
};

}