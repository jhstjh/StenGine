#include "Graphics/Animation/Animation.h"
#include "Engine/EventSystem.h"

#define INTERPOLATE_ANIMATION 0

namespace StenGine
{

TSQ AnimationNode::UpdateAnimation(Timer::Seconds playbackTime)
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

#if INTERPOLATE_ANIMATION
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
#endif

	return { pos, rot, scal };
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