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
	bool HasCurrentClip() { return mBledingClips.size() != 0; }
	void SetCurrentClip(const std::string &name, float blendTime = 0.f);
	void SetPositionDrivenNodeName(const std::string &name) { mPositionDrivenNodeName = name; }
	void SetPlaySpeed(float speed) { mPlaySpeed = speed; }

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

	struct BlendingClip
	{
		AnimationClip* clip;
		Timer::Seconds playbackTime;
		Timer::Seconds totalBlendTime;
		Vec3		   posDrivenNodePos;
		bool		   validDriven;
		Vec3		   posDiff;
		Timer::Seconds currBlendTime;

		TransformTSQMap transformTSQ;

		BlendingClip(
			AnimationClip* _clip,
			Timer::Seconds _playbackTime,
			Timer::Seconds _totalBlendTime,
			Vec3		   _posDrivenNodePos,
			bool		   _validDriven
		)
			: clip(_clip)
			, playbackTime(_playbackTime)
			, totalBlendTime(_totalBlendTime)
			, posDrivenNodePos(_posDrivenNodePos)
			, validDriven(_validDriven)
			, currBlendTime(0.f)
			, posDiff(0.f, 0.f, 0.f)
		{
			printf("New Clip Added: %s\n", clip->name.c_str());
		}

		~BlendingClip()
		{
			printf("Clip Removed: %s\n", clip->name.c_str());
		}
	};

	Animation*     mAnimation{ nullptr };
	bool		   mPlay{ true };

	std::unordered_map<std::string, AnimationClip*> mAnimationClips;
	std::deque<BlendingClip> mBledingClips;

	std::string	   mPositionDrivenNodeName;
	bool		   mValidDriven{ false };
	float		   mPlaySpeed{ 1.0f };
};

}