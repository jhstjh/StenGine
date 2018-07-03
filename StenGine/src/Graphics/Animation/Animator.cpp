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
		mPlaybackTime = 0;
		mValidDriven = false;
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
	bool warpAround = false;
	if (mPlaybackTime > (mCurrentClip->endFrame - mCurrentClip->startFrame) / mAnimation->GetFrameRate())
	{
		warpAround = true;
	}
	mPlaybackTime = fmod(mPlaybackTime, (mCurrentClip->endFrame - mCurrentClip->startFrame) / mAnimation->GetFrameRate());

	for (auto &node : mAnimation->m_animations)
	{
		mTransformMatrices[node.first] = node.second.UpdateAnimation(mPlaybackTime + mCurrentClip->startFrame / mAnimation->GetFrameRate());

		if (node.first == mPositionDrivenNodeName)
		{
			Vec3 pos = MatrixHelper::GetPosition(mTransformMatrices[node.first]);
			Vec3 diff = pos - mLastPosDrivenNodePos;

			if (warpAround)
			{
				Vec3 startPos = MatrixHelper::GetPosition(node.second.UpdateAnimation(mCurrentClip->startFrame / mAnimation->GetFrameRate()));
				Vec3 endPos = MatrixHelper::GetPosition(node.second.UpdateAnimation(mCurrentClip->endFrame / mAnimation->GetFrameRate()));
				diff += (endPos - startPos);
			}

			if (mValidDriven)
			{
				mParent->GetTransform()->MoveForward(diff.z());
				mParent->GetTransform()->MoveRight(diff.x());

				MatrixHelper::SetPosition(mTransformMatrices[node.first], 0.f, pos.y(), 0.f);
			}

			mLastPosDrivenNodePos = pos;
			mValidDriven = true;
		}
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
