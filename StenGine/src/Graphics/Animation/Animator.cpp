#include <algorithm>

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
	mBledingClips.clear();

	for (auto& entry : mAnimationClips)
	{
		delete entry.second;
	}

	mAnimationClips.clear();
}

const Mat4 Animator::GetTransform(std::string name) const
{
	if (mBledingClips.size())
	{
		auto &currClip = mBledingClips.back();
		auto find = currClip.transformTSQ.find(name);
		if (find != currClip.transformTSQ.end())
		{
			return find->second.ToMat4();
		}
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
		if (mBledingClips.size() == 0)
		{
			mBledingClips.emplace_back(entry->second, 0.f, 0.f, Vec3(0.f, 0.f, 0.f), false);
		}
		else
		{
			auto& prevClip = mBledingClips.back();
			prevClip.totalBlendTime = blendTime;
			prevClip.currBlendTime = 0.f;
			mBledingClips.emplace_back(entry->second, 0.f, 0.f, Vec3(0.f, 0.f, 0.f), false);
			mBledingClips.back().transformTSQ = prevClip.transformTSQ;
		}

		mValidDriven = false;
		mPlaySpeed = 1.f; // reset playspeed for new clip
	}
}

void Animator::UpdateClip(const AnimationClip* clip, Timer::Seconds &playbackTime, TransformTSQMap &transformTSQMap, Vec3 &prevDrivenNodePos, bool &validDriven, Vec3 &drivenNodePosDiff)
{
	auto dt = Timer::GetDeltaTime();
	playbackTime += dt * mPlaySpeed;
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
	if (!mAnimation || !mPlay || mBledingClips.size() == 0)
	{
		return;
	}

	const auto dt = Timer::GetDeltaTime();

	auto UpdateClipFunc = [dt, this](BlendingClip &blendClip)
	{
		UpdateClip(blendClip.clip, blendClip.playbackTime, blendClip.transformTSQ, blendClip.posDrivenNodePos, blendClip.validDriven, blendClip.posDiff);
		blendClip.currBlendTime += dt;
	};

	UpdateClipFunc(mBledingClips.front());
	
	for (size_t i = 1; i < mBledingClips.size(); i++)
	{
		auto &prevClip = mBledingClips[i - 1];
		auto &currClip = mBledingClips[i];

		UpdateClipFunc(currClip);
		float percent = static_cast<float>(prevClip.currBlendTime / prevClip.totalBlendTime);
		percent = percent > 1.f ? 1.f : percent;

		for (auto& tsq : currClip.transformTSQ)
		{
			tsq.second.pos = Vec3::Lerp(prevClip.transformTSQ[tsq.first].pos, tsq.second.pos, percent);
			tsq.second.rot = Quat::Slerp(prevClip.transformTSQ[tsq.first].rot, tsq.second.rot, percent);
			tsq.second.scale = Vec3::Lerp(prevClip.transformTSQ[tsq.first].scale, tsq.second.scale, percent);
		}

		currClip.posDiff = Vec3::Lerp(prevClip.posDiff, currClip.posDiff, percent);
	}

	if (mBledingClips.size() > 1)
	{
		auto it = mBledingClips.crbegin() + 1;
		for (;it != mBledingClips.crend(); it++)
		{
			if (it->currBlendTime >= it->totalBlendTime)
			{
				break;
			}
		}

		if (it != mBledingClips.crend())
		{
			mBledingClips.erase(mBledingClips.begin(), mBledingClips.begin() + std::distance(it, mBledingClips.crend()));
		}
	}

	auto &currentClip = mBledingClips.back();
	if (currentClip.posDiff != Vec3{0.f, 0.f, 0.f})
	{
		auto transform = mParent->GetTransform();
		transform->MoveForward(currentClip.posDiff.z());
		transform->MoveRight(currentClip.posDiff.x());
	}
}

void Animator::DrawMenu()
{
	if (ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (mAnimation)
		{
			if (mBledingClips.size())
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

				auto &currClip = mBledingClips.back();
				float clipLength = (currClip.clip->endFrame - currClip.clip->startFrame) / mAnimation->GetFrameRate();

				ImGui::Text("Current Clip: %s", currClip.clip->name.c_str());
				ImGui::Text("%f/%f", currClip.playbackTime, clipLength);
				ImGui::ProgressBar(currClip.playbackTime / clipLength);
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
