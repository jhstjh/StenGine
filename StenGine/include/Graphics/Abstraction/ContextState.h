#pragma once

#include <stdint.h>
#include <functional>

namespace StenGine
{

#define DEFINE_OPERATOR(T) \
bool operator==(const T &x) const \
{ \
	return (value == x.value); \
} \
T& operator=(T x) \
{ \
	value = x.value; \
	return *this; \
}

#pragma warning (push)
#pragma warning (disable:4201) //  nonstandard extension used: nameless struct/union

struct BlendState
{
	enum class Blend : uint32_t
	{
		BLEND_ZERO = 1,
		BLEND_ONE = 2,
		BLEND_SRC_COLOR = 3,
		BLEND_INV_SRC_COLOR = 4,
		BLEND_SRC_ALPHA = 5,
		BLEND_INV_SRC_ALPHA = 6,
		BLEND_DEST_ALPHA = 7,
		BLEND_INV_DEST_ALPHA = 8,
		BLEND_DEST_COLOR = 9,
		BLEND_INV_DEST_COLOR = 10,
		BLEND_SRC_ALPHA_SAT = 11,
		// BLEND_BLEND_FACTOR = 14,
		// BLEND_INV_BLEND_FACTOR = 15,
		// BLEND_SRC1_COLOR = 16,
		// BLEND_INV_SRC1_COLOR = 17,
		// BLEND_SRC1_ALPHA = 18,
		// BLEND_INV_SRC1_ALPHA = 19
	};

	enum class BlendOp : uint32_t
	{
		BLEND_OP_ADD = 1,
		BLEND_OP_SUBTRACT = 2,
		BLEND_OP_REV_SUBTRACT = 3,
		BLEND_OP_MIN = 4,
		BLEND_OP_MAX = 5
	};

	union
	{
		struct 
		{
			uint32_t	blendEnable		: 1;
			uint32_t	index			: 3;
			Blend		srcBlend		: 4;
			Blend		destBlend		: 4;
			BlendOp		blendOpColor	: 4;
			//Blend	srcBlendAlpha;
			//Blend	destBlendAlpha;
			//BlendOp blendOpAlpha;
			uint8_t		renderTargetWriteMask : 4;
		};

		uint32_t	value = 0;
	};

	DEFINE_OPERATOR(BlendState)
};

struct DepthState
{
	enum class DepthFunc : uint32_t
	{
		NEVER = 1,
		LESS = 2,
		EQUAL = 3,
		LESS_EQUAL = 4,
		GREATER = 5,
		NOT_EQUAL = 6,
		GREATER_EQUAL = 7,
		ALWAYS = 8
	};

	union 
	{
		struct  
		{
			uint32_t	  depthCompEnable : 1;
			uint32_t      depthWriteEnable : 1;
			DepthFunc	  depthFunc : 4;
		};

		uint32_t	value;
	};

	DepthState()
	{
		depthCompEnable = 1;
		depthWriteEnable = 1;
		depthFunc = DepthFunc::LESS_EQUAL;
	}

	DEFINE_OPERATOR(DepthState)
};

struct ScissorState
{
	bool	scissorTestEnabled = false;
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
};

struct RasterizerState
{
	enum class FrontFace : uint32_t
	{
		CW = 1,
		CCW = 2,
	};

	enum class CullType : uint32_t
	{
		FRONT = 1,
		BACK = 2,
	};

	union
	{
		struct  
		{
			uint32_t	cullFaceEnabled : 1;
			FrontFace	frontFace : 2;
			CullType	cullType : 2;
		};

		uint32_t value;
	};

	RasterizerState()
	{
		cullFaceEnabled = true;
		frontFace = FrontFace::CW;
		cullType = CullType::BACK;
	}

	DEFINE_OPERATOR(RasterizerState)
};

}

#pragma warning (pop)

namespace std
{
template <>
struct hash<StenGine::BlendState>
{
	size_t operator()(StenGine::BlendState const & x) const noexcept
	{
		return (std::hash<uint32_t>()(x.value));
	}
};

template <>
struct hash<StenGine::DepthState>
{
	size_t operator()(StenGine::DepthState const & x) const noexcept
	{
		return (std::hash<uint32_t>()(x.value));
	}
};

template <>
struct hash<StenGine::RasterizerState>
{
	size_t operator()(StenGine::RasterizerState const & x) const noexcept
	{
		return (std::hash<uint32_t>()(x.value));
	}
};
}