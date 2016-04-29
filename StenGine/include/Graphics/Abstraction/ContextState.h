#pragma once

#include <stdint.h>

namespace StenGine
{

struct BlendState
{
	enum class Blend
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

	enum class BlendOp
	{
		BLEND_OP_ADD = 1,
		BLEND_OP_SUBTRACT = 2,
		BLEND_OP_REV_SUBTRACT = 3,
		BLEND_OP_MIN = 4,
		BLEND_OP_MAX = 5
	};

	bool	blendEnable = false;
	uint32_t index;
	Blend	srcBlend;
	Blend	destBlend;
	BlendOp blendOpColor;
	//Blend	srcBlendAlpha;
	//Blend	destBlendAlpha;
	//BlendOp blendOpAlpha;
	uint8_t renderTargetWriteMask;
};

struct DepthState
{
	enum class DepthFunc
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

	bool	  depthCompEnable = true;
	bool      depthWriteEnable = true;
	DepthFunc depthFunc = DepthFunc::LESS_EQUAL;
};

struct ScissorState
{
	bool	scissorTestEnabled = false;
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
};

struct CullState
{
	enum class FrontFace
	{
		CW = 1,
		CCW = 2,
	};

	enum class CullType
	{
		FRONT = 1,
		BACK = 2,
	};

	bool		cullFaceEnabled = true;
	FrontFace	frontFace = FrontFace::CW;
	CullType	cullType = CullType::BACK;
};

}