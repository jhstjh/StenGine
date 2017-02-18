#pragma once

#include "Scene/Component.h"
#include "Scene/Savable.h"
#include "Graphics/D3DIncludes.h"
#include "System/AlignedClass.h"

namespace StenGine
{

class Component;

class Transform : public Component, public AlignedClass<16>, public Savable
{
public:
	Transform(float tx = 0, float ty = 0, float tz = 0,
			  float rx = 0, float ry = 0, float rz = 0,
			  float sx = 1, float sy = 1, float sz = 1);

	const XMFLOAT4X4*		GetWorldTransform();
	XMFLOAT3				GetPosition() const;
	void					RotateAroundY(float radius);

	virtual void			DrawMenu();
	tinyxml2::XMLElement*	Save(tinyxml2::XMLDocument &doc) const override;

private:
	Transform*	mParent;

	XMVECTOR	mPosition;
	XMVECTOR	mRotation;
	XMVECTOR	mScale;

	XMFLOAT4X4	mWorldTransform;

	const float mTx, mTy, mTz, mRx, mRy, mRz, mSx, mSy, mSz; // initial transform
};

}