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

	inline void SetAnimation(Animation* anim)
	{
		mAnimation = anim;
	}

	const Mat4 GetTransform(std::string str) const;

	void DrawMenu() override;

private:
	void UpdateAnimation();

	Animation*     mAnimation{ nullptr };
	Timer::Seconds mPlaybackTime{ 0.0 };
	bool		   mPlay{ true };
	std::unordered_map<std::string, Mat4> mTransformMatrices;
};

}