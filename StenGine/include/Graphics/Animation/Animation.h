#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Scene/Transform.h"
#include "Math/MathHelper.h"

namespace StenGine
{

const XMVECTOR VEC3ZERO = XMLoadFloat3(&XMFLOAT3(0, 0, 0));
const XMVECTOR QUATIDENT = XMQuaternionIdentity();
const float FRAME_RATE = 30.f;

struct AnimationNode
{
	std::vector<XMVECTOR> position;
	std::vector<XMVECTOR> rotation;
	std::vector<XMVECTOR> scale;

	std::vector<float> positionTime;
	std::vector<float> rotationTime;
	std::vector<float> scaleTime;

	uint32_t positionIndex = 0;
	uint32_t rotationIndex = 0;
	uint32_t scaleIndex = 0;

	void UpdateAnimation()
	{
		float dt = Timer::GetDeltaTime();
		playbackTime += dt * FRAME_RATE;

		if (playbackTime > positionTime[positionIndex])
		{
			positionIndex = (positionIndex + 1) % positionTime.size();
			dirty = true;
		}
		if (playbackTime > rotationTime[rotationIndex])
		{
			rotationIndex = (rotationIndex + 1) % rotationTime.size();
			dirty = true;
		}
		if (playbackTime > scaleTime[scaleIndex])
		{
			scaleIndex = (scaleIndex + 1) % scaleTime.size();
			dirty = true;
		}
	}

	XMMATRIX GetTransform()
	{
		if (dirty)
		{
			transformMatrix = XMMatrixTransformation(VEC3ZERO, QUATIDENT, scale[scaleIndex], VEC3ZERO, rotation[rotationIndex], position[positionIndex]);
			dirty = false;
		}
		return transformMatrix;
	}

	float playbackTime = 0;
	XMMATRIX transformMatrix = XMMatrixIdentity();
	bool dirty = true;
	uint32_t length = 0;
};

class Animation {
public:
	std::unordered_map<std::string, AnimationNode> m_animations;

	Animation();

	void Update()
	{
		for (auto &node : m_animations)
		{
			node.second.UpdateAnimation();
		}
	}

	XMMATRIX GetTransform(std::string name)
	{
		auto find = m_animations.find(name);
		if (find != m_animations.end())
		{
			return find->second.GetTransform();
		}
		return IDENTITY_MAT;
	}
};

}