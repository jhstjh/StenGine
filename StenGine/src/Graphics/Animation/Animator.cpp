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

Animator::~Animator()
{
	for (auto& entry : mAnimationClips)
	{
		delete entry.second;
	}

	mAnimationClips.clear();
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

void Animator::CreateClip(float startFrame, float endFrame, const std::string & name)
{
	assert(mAnimationClips.find(name) == mAnimationClips.end());
	mAnimationClips[name] = new AnimationClip{ startFrame, endFrame, name };
}

void Animator::SetCurrentClip(const std::string & name)
{
	auto entry = mAnimationClips.find(name);
	if (entry != mAnimationClips.end())
	{
		mCurrentClip = entry->second;
	}
}

void Animator::UpdateAnimation()
{
	if (!mAnimation || !mPlay || !mCurrentClip)
	{
		return;
	}

	auto dt = Timer::GetDeltaTime();
	mPlaybackTime += dt;
	mPlaybackTime = fmod(mPlaybackTime, (mCurrentClip->endFrame - mCurrentClip->startFrame) / mAnimation->GetFrameRate());

	for (auto &node : mAnimation->m_animations)
	{
		mTransformMatrices[node.first] = node.second.UpdateAnimation(mPlaybackTime + mCurrentClip->startFrame / mAnimation->GetFrameRate());
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
			float clipLength = (mCurrentClip->endFrame - mCurrentClip->startFrame) / mAnimation->GetFrameRate();
			ImGui::Text("Current Clip: %s", mCurrentClip->name.c_str());
			ImGui::Text("%f/%f", mPlaybackTime, clipLength);
			ImGui::ProgressBar(mPlaybackTime / clipLength);
		}
	}
}

}
