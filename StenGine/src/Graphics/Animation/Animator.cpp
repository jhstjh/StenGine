#include "Engine/EventSystem.h"
#include "Graphics/Animation/Animation.h"
#include "Graphics/Animation/Animator.h"
#include "Scene/GameObject.h"
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
	auto find = mTransformTSQ.find(name);
	if (find != mTransformTSQ.end())
	{
		return find->second.ToMat4();
	}
	return Mat4::Identity();
}

void Animator::CreateClip(float startFrame, float endFrame, const std::string & name)
{
	assert(mAnimationClips.find(name) == mAnimationClips.end());
	mAnimationClips[name] = new AnimationClip{ startFrame, endFrame, name };
}

void Animator::SetCurrentClip(const std::string & name, float blendTime /*= 0.f*/)
{
	auto entry = mAnimationClips.find(name);
	if (entry != mAnimationClips.end())
	{
		mPrevPlaybackTime = mPlaybackTime;
		mTotalBlendTime = blendTime;
		mCurrBlendTime = 0.0;
		mPreviousClip = mCurrentClip;
		mPrevLastPosDrivenNodePos = mLastPosDrivenNodePos;
		mPrevValidDriven = mValidDriven;

		mPlaybackTime = 0;
		mValidDriven = false;
		mCurrentClip = entry->second;
	}
}

void Animator::UpdateClip(const AnimationClip* clip, Timer::Seconds &playbackTime, TransformTSQMap &transformTSQMap, Vec3 &prevDrivenNodePos, bool &validDriven, Vec3 &drivenNodePosDiff)
{
	auto dt = Timer::GetDeltaTime();
	playbackTime += dt;
	bool warpAround = false;
	if (playbackTime > (clip->endFrame - clip->startFrame) / mAnimation->GetFrameRate())
	{
		warpAround = true;
	}
	playbackTime = fmod(playbackTime, (clip->endFrame - clip->startFrame) / mAnimation->GetFrameRate());
	drivenNodePosDiff = { 0, 0, 0 };

	for (auto &node : mAnimation->m_animations)
	{
		transformTSQMap[node.first] = node.second.UpdateAnimation(playbackTime + clip->startFrame / mAnimation->GetFrameRate());

		if (node.first == mPositionDrivenNodeName)
		{
			Vec3 pos = transformTSQMap[node.first].pos;
			Vec3 diff = pos - prevDrivenNodePos;

			if (warpAround)
			{
				TSQ start = node.second.UpdateAnimation(clip->startFrame / mAnimation->GetFrameRate());
				TSQ end = node.second.UpdateAnimation(clip->endFrame / mAnimation->GetFrameRate());

				Vec3 startPos = start.pos;
				Vec3 endPos = end.pos;
				diff += (endPos - startPos);
			}

			if (validDriven)
			{
				drivenNodePosDiff = diff;
				transformTSQMap[node.first].pos = { 0.f, pos.y(), 0.f };
			}

			prevDrivenNodePos = pos;
			validDriven = true;
		}
	}
}

void Animator::UpdateAnimation()
{
	if (!mAnimation || !mPlay || !mCurrentClip)
	{
		return;
	}

	Vec3 posDiff;
	UpdateClip(mCurrentClip, mPlaybackTime, mTransformTSQ, mLastPosDrivenNodePos, mValidDriven, posDiff);

	auto dt = Timer::GetDeltaTime();
	mCurrBlendTime += dt;
	
	if (mCurrBlendTime < mTotalBlendTime && mPreviousClip)
	{
		Vec3 prevPosDiff;
		UpdateClip(mPreviousClip, mPrevPlaybackTime, mPrevTransformTSQ, mPrevLastPosDrivenNodePos, mPrevValidDriven, prevPosDiff);

		for (auto& tsq : mTransformTSQ)
		{
			tsq.second.pos = Vec3::Lerp(mPrevTransformTSQ[tsq.first].pos, tsq.second.pos, static_cast<float>(mCurrBlendTime / mTotalBlendTime));
			tsq.second.rot = Quat::Slerp(mPrevTransformTSQ[tsq.first].rot, tsq.second.rot, static_cast<float>(mCurrBlendTime / mTotalBlendTime));
			tsq.second.scale = Vec3::Lerp(mPrevTransformTSQ[tsq.first].scale, tsq.second.scale, static_cast<float>(mCurrBlendTime / mTotalBlendTime));
		}

		posDiff = Vec3::Lerp(prevPosDiff, posDiff, static_cast<float>(mCurrBlendTime / mTotalBlendTime));
	}

	if (posDiff != Vec3{0.f, 0.f, 0.f})
	{
		auto transform = mParent->GetTransform();
		transform->MoveForward(posDiff.z());
		transform->MoveRight(posDiff.x());
	}
}

void Animator::DrawMenu()
{
	if (ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (mAnimation)
		{
			if (mCurrentClip)
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

				float clipLength = (mCurrentClip->endFrame - mCurrentClip->startFrame) / mAnimation->GetFrameRate();

				ImGui::Text("Current Clip: %s", mCurrentClip->name.c_str());
				ImGui::Text("%f/%f", mPlaybackTime, clipLength);
				ImGui::ProgressBar(mPlaybackTime / clipLength);
			}

			if (ImGui::TreeNode("Clips"))
			{
				for (auto& clip : mAnimationClips)
				{
					if (ImGui::TreeNode(clip.first.c_str()))
					{
						ImGui::InputFloat("Start Frame", &clip.second->startFrame);
						ImGui::InputFloat("End Frame", &clip.second->endFrame);
						if (ImGui::Button("Set Current"))
						{
							SetCurrentClip(clip.first);
						}

						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			}
		}
	}
}

}
