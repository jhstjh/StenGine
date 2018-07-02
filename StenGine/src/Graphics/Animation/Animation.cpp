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

	Mat4 transformMatrix = Mat4::Identity();

	Mat4 t = Mat4::FromTranslationVector(position[positionIndex]);
	Mat4 q = rotation[rotationIndex].ToMatrix4();
	Mat4 s = Mat4::FromScaleVector(scale[scaleIndex]);

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