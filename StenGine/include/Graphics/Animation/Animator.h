#pragma once

#include <unordered_map>
#include "Graphics/Animation/Animation.h"
#include "Math/MathDefs.h"
#include "Scene/Component.h"

namespace StenGine
{

class Animation;

class Animator : public Component
{
public:
	Animator();
	virtual ~Animator();

	inline void SetAnimation(Animation* anim)
	{
		mAnimation = anim;
	}

	const Mat4 GetTransform(std::string str) const;

	void CreateClip(float startFrame, float endFrame, const std::string &name);
	bool HasCurrentClip() { return mCurrentClip != nullptr; }
	void SetCurrentClip(const std::string &name, float blendTime = 0.f);
	void SetPositionDrivenNodeName(const std::string &name) { mPositionDrivenNodeName = name; }

	void DrawMenu() override;

private:
	using TransformTSQMap = std::unordered_map<std::string, TSQ>;

	struct AnimationClip
	{
		float startFrame;
		float endFrame;
		std::string name;
	};

	void UpdateClip(const AnimationClip* clip, Timer::Seconds &playbackTime, TransformTSQMap &transformMtxMap, Vec3 &prevDrivenNodePos, bool &validDriven, Vec3 &drivenNodePosDiff);
	void UpdateAnimation();

	Animation*     mAnimation{ nullptr };
	AnimationClip* mPreviousClip{ nullptr };
	AnimationClip* mCurrentClip{ nullptr };
	Timer::Seconds mPrevPlaybackTime{ 0.0 };
	Timer::Seconds mPlaybackTime{ 0.0 };
	Timer::Seconds mTotalBlendTime{ 0.0 };
	Timer::Seconds mCurrBlendTime{ 0.0 };
	bool		   mPlay{ true };

	TransformTSQMap mPrevTransformTSQ;
	TransformTSQMap mTransformTSQ;
	std::unordered_map<std::string, AnimationClip*> mAnimationClips;

	std::string	   mPositionDrivenNodeName;
	Vec3		   mLastPosDrivenNodePos{ 0, 0, 0 };
	Vec3		   mPrevLastPosDrivenNodePos{ 0, 0, 0 };
	bool		   mValidDriven{ false };
	bool		   mPrevValidDriven{ false };
};

}