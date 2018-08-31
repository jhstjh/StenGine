#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Scene/Transform.h"
#include "Math/MathHelper.h"

namespace StenGine
{

const float FRAME_RATE = 30.f;

struct TSQ
{
	Vec3 pos;
	Quat rot;
	Vec3 scale;

	Mat4 ToMat4() const
	{
		Mat4 t = Mat4::FromTranslationVector(pos);
		Mat4 q = rot.ToMatrix4();
		Mat4 s = Mat4::FromScaleVector(scale);
		return t * s * q;
	}
};

struct AnimationNode
{
	std::vector<Vec3> position;
	std::vector<Quat> rotation;
	std::vector<Vec3> scale;

	std::vector<float> positionTime;
	std::vector<float> rotationTime;
	std::vector<float> scaleTime;

	TSQ UpdateAnimation(Timer::Seconds playbackTime);

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