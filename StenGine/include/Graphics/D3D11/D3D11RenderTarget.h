#pragma once

#include <vector>
#include "Graphics/D3DIncludes.h"
#include "Graphics/Color.h"

namespace StenGine
{

struct RenderTarget
{
	std::vector<ID3D11RenderTargetView*> rtvs;
	std::vector<SGColors> clearColor;
	ID3D11DepthStencilView* dsv = nullptr;

	void SetRenderTarget(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->OMSetRenderTargets((UINT)rtvs.size(), rtvs.data(), dsv);
	}

	void ClearColor(ID3D11DeviceContext* deviceContext)
	{
		for (uint32_t i = 0; i < rtvs.size(); i++)
		{
			deviceContext->ClearRenderTargetView(rtvs[i], reinterpret_cast<const float*>(&clearColor[i]));
		}
	}

	void ClearDepth(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
};

}