#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Scene/Transform.h"
#include "Math/MathDefs.h"
#include "Math/MathHelper.h"

namespace StenGine
{

const Vec3 VEC3ZERO{ 0, 0, 0 };
const Quat QUATIDENT = Quat::identity;
const float FRAME_RATE = 30.f;

struct AnimationNode
{
	std::vector<Vec3> position;
	std::vector<Quat> rotation;
	std::vector<Vec3> scale;

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

	Mat4 GetTransform()
	{
		if (dirty)
		{
			Mat4 t = Mat4::FromTranslationVector(position[positionIndex]);
			Mat4 q = rotation[rotationIndex].ToMatrix4();
			Mat4 s = Mat4::FromScaleVector(scale[scaleIndex]);

			transformMatrix = t * s * q;

			dirty = false;
		}
		return transformMatrix;
	}

	float playbackTime = 0;
	Mat4 transformMatrix = Mat4::Identity();
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

	Mat4 GetTransform(std::string name)
	{
		auto find = m_animations.find(name);
		if (find != m_animations.end())
		{
			return find->second.GetTransform();
		}
		return Mat4::Identity();
	}
};

}