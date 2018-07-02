#include "Engine/EventSystem.h"
#include "Graphics/Animation/Animation.h"
#include "Graphics/Animation/Animator.h"
#include "Utility/Timer.h"

namespace StenGine
{

Animator::Animator()
{
	EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::UPDATE_ANIMATION, [this]() { UpdateAnimation(); });
}

const Mat4 Animator::GetTransform(std::string name) const
{
	auto find = mTransformMatrices.find(name);
	if (find != mTransformMatrices.end())
	{
		return find->second;
	}
	return Mat4::Identity();
}

void Animator::UpdateAnimation()
{
	if (!mAnimation || !mPlay)
	{
		return;
	}

	auto dt = Timer::GetDeltaTime();
	mPlaybackTime += dt;
	mPlaybackTime = fmod(mPlaybackTime, mAnimation->GetLengthInSec());

	for (auto &node : mAnimation->m_animations)
	{
		mTransformMatrices[node.first] = node.second.UpdateAnimation(mPlaybackTime);
	}
}

void Animator::DrawMenu()
{
	if (ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (mPlay)
		{
			if (ImGui::Button("Stop Animation"))
			{
				mPlay = false;
			}
		}
		else
		{
			if (ImGui::Button("Start Animation"))
			{
				mPlay = true;
			}
		}

		if (mAnimation)
		{
			ImGui::Text("%f/%f", mPlaybackTime, mAnimation->GetLengthInSec());
			ImGui::ProgressBar(mPlaybackTime / mAnimation->GetLengthInSec());
		}
	}
}

}
