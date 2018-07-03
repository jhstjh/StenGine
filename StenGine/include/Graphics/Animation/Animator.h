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

	void SetCurrentClip(const std::string &name);

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
	AnimationClip* mCurrentClip;
	Timer::Seconds mPlaybackTime{ 0.0 };
	bool		   mPlay{ true };
	std::unordered_map<std::string, Mat4> mTransformMatrices;
	std::unordered_map<std::string, AnimationClip*> mAnimationClips;
};

}