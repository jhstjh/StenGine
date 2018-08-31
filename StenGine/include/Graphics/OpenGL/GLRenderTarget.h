#pragma once

#include "Graphics/Abstraction/RenderTarget.h"
#include "glew.h"

namespace StenGine
{

class GLRenderTarget : public RenderTargetImpl
{
	GLuint mFramebuffer;

	void SetRenderTarget(void* /*deviceContext*/) final
	{
	}

	void ClearColor(void* /*deviceContext*/) final
	{
	}

	void ClearDepth(void* /*deviceContext*/) final
	{
	}

	void Set(uint32_t framebuffer) final
	{
		mFramebuffer = framebuffer;
	}

	uint32_t Get() final
	{
		return mFramebuffer;
	}
};

}