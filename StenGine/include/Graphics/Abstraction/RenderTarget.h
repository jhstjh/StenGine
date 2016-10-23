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


class RenderTarget
{
public:
	RenderTarget();
	// D3D11 only
	void AddRenderTarget(void* renderTarget);
	void AddClearColor(SGColors color);
	void AssignDepthStencil(void* depthStencil);

	void SetRenderTarget(void* deviceContext);
	void ClearColor(void* deviceContext);
	void ClearDepth(void* deviceContext);

	// GL only
	void Set(uint32_t framebuffer);
	uint32_t Get();
private:
	std::unique_ptr<RenderTargetImpl> mImpl;
};

}