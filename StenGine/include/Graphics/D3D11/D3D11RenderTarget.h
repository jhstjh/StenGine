#pragma once

#include <vector>
#include "Graphics/D3DIncludes.h"
#include "Graphics/Color.h"
#include "Graphics/Abstraction/RenderTarget.h"

namespace StenGine
{

class D3D11RenderTarget : public RenderTargetImpl
{
	std::vector<ID3D11RenderTargetView*> rtvs;
	std::vector<SGColors> clearColor;
	ID3D11DepthStencilView* dsv = nullptr;

	void AddRenderTarget(void* renderTarget) 
	{
		rtvs.push_back(reinterpret_cast<ID3D11RenderTargetView*>(renderTarget));
	}

	void AddClearColor(SGColors color)
	{
		clearColor.push_back(color);
	}

	void AssignDepthStencil(void* depthStencil) 
	{
		dsv = reinterpret_cast<ID3D11DepthStencilView*>(depthStencil);
	}

	void SetRenderTarget(void* deviceContext) final
	{
		reinterpret_cast<ID3D11DeviceContext*>(deviceContext)->OMSetRenderTargets((UINT)rtvs.size(), rtvs.data(), dsv);
	}

	void ClearColor(void* deviceContext) final
	{
		for (uint32_t i = 0; i < rtvs.size(); i++)
		{
			reinterpret_cast<ID3D11DeviceContext*>(deviceContext)->ClearRenderTargetView(rtvs[i], reinterpret_cast<const float*>(&clearColor[i]));
		}
	}

	void ClearDepth(void* deviceContext) final
	{
		reinterpret_cast<ID3D11DeviceContext*>(deviceContext)->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
};

}