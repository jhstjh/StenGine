#include "Graphics/Animation/Animation.h"
#include "Engine/EventSystem.h"

namespace StenGine
{

Mat4 AnimationNode::UpdateAnimation(Timer::Seconds playbackTime)
{
	float playbackFrame = static_cast<float>(playbackTime) * FRAME_RATE;

	auto findIdx = [playbackFrame](std::vector<float>& timeVector)
	{
		uint32_t index = 0;
		auto positionIt = std::lower_bound(timeVector.begin(), timeVector.end(), playbackFrame);
		if (positionIt != timeVector.end())
		{
			index = std::distance(timeVector.begin(), positionIt);
		}
		return index;
	};

	uint32_t positionIndex = findIdx(positionTime);
	uint32_t rotationIndex = findIdx(rotationTime);
	uint32_t scaleIndex = findIdx(scaleTime);

	auto pos = position[positionIndex];
	auto rot = rotation[rotationIndex];
	auto scal = scale[scaleIndex];

	if (positionIndex < position.size() - 1)
	{
		float percent = (playbackFrame - positionTime[positionIndex]) / (positionTime[positionIndex + 1] - positionTime[positionIndex]);
		pos = Vec3::Lerp(pos, position[positionIndex + 1], percent);
	}

	if (rotationIndex < rotation.size() - 1)
	{
		float percent = (playbackFrame - rotationTime[rotationIndex]) / (rotationTime[rotationIndex + 1] - rotationTime[rotationIndex]);
		rot = Quat::Slerp(rot, rotation[rotationIndex + 1], percent);
	}

	if (scaleIndex < scale.size() - 1)
	{
		float percent = (playbackFrame - scaleTime[scaleIndex]) / (scaleTime[scaleIndex + 1] - scaleTime[scaleIndex]);
		scal = Vec3::Lerp(scal, scale[scaleIndex + 1], percent);
	}

	Mat4 transformMatrix = Mat4::Identity();

	Mat4 t = Mat4::FromTranslationVector(pos);
	Mat4 q = rot.ToMatrix4();
	Mat4 s = Mat4::FromScaleVector(scal);

	transformMatrix = t * s * q;

	return transformMatrix;
}

void Animation::DoneReading()
{
	for (auto & node : m_animations)
	{
		if (node.second.length > m_length)
		{
			m_length = node.second.length;
		}
	}
}
}