#pragma once

#include <unordered_map>
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
	void SetCurrentClip(const std::string &name);
	void SetPositionDrivenNodeName(const std::string &name) { mPositionDrivenNodeName = name; }

	void DrawMenu() override;

private:
	struct AnimationClip
	{
		float startFrame;
		float endFrame;
		std::string name;
	};

	void UpdateAnimation();

	Animation*     mAnimation{ nullptr };
	AnimationClip* mCurrentClip{ nullptr };
	Timer::Seconds mPlaybackTime{ 0.0 };
	bool		   mPlay{ true };
	std::unordered_map<std::string, Mat4> mTransformMatrices;
	std::unordered_map<std::string, AnimationClip*> mAnimationClips;

	std::string	   mPositionDrivenNodeName;
	Vec3		   mLastPosDrivenNodePos{ 0, 0, 0 };
	bool		   mValidDriven{ false };
};

}