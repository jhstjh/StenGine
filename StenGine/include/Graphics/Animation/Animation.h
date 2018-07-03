#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Scene/Transform.h"
#include "Math/MathDefs.h"
#include "Math/MathHelper.h"

namespace StenGine
{

const float FRAME_RATE = 30.f;

struct AnimationNode
{
	std::vector<Vec3> position;
	std::vector<Quat> rotation;
	std::vector<Vec3> scale;

	std::vector<float> positionTime;
	std::vector<float> rotationTime;
	std::vector<float> scaleTime;

	Mat4 UpdateAnimation(Timer::Seconds playbackTime);

	float length{ 0.f };
};

class Animation {
public:
	std::unordered_map<std::string, AnimationNode> m_animations;
	float m_length{ 0.f };

	void DoneReading();
	float GetLengthInSec() { return m_length / FRAME_RATE; }
	float GetLengthInFrame() { return m_length; }
	float GetFrameRate() { return FRAME_RATE; }
};

}