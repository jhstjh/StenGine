#pragma once

#include <memory>
#include "Graphics/Color.h"

namespace StenGine
{

class RenderTargetImpl
{
public:
	virtual ~RenderTargetImpl() = default;
	virtual void AddRenderTarget(void* renderTarget) {};
	virtual void AddClearColor(SGColors color) {};
	virtual void AssignDepthStencil(void* depthStencil) {};
	virtual void SetRenderTarget(void* deviceContext) = 0;
	virtual void ClearColor(void* deviceContext) = 0;
	virtual void ClearDepth(void* deviceContext) = 0;
	virtual void Set(uint32_t framebuffer) {};
	virtual uint32_t Get() { return 0; };
};

using RenderTarget = std::shared_ptr<RenderTargetImpl>;

}