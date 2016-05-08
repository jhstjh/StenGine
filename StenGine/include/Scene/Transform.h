#pragma once

#include "Scene/Component.h"
#include "Graphics/D3DIncludes.h"
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

	const XMFLOAT4X4* GetWorldTransform();
	XMFLOAT3 GetPosition();
	void RotateAroundY(float radius);

	virtual void DrawMenu();

private:
	Transform* m_parent;

	XMVECTOR m_position;
	XMVECTOR m_rotation;
	XMVECTOR m_scale;

	XMFLOAT4X4 m_worldTransform;
};

}