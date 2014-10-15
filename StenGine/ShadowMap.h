#ifndef __SHADOW_MAP__
#define __SHADOW_MAP__

#include "D3DIncludes.h"

class ShadowMap {
private:
	UINT m_width;
	UINT m_height;

	ID3D11ShaderResourceView* m_depthSRV;
	ID3D11DepthStencilView* m_depthDSV;
	D3D11_VIEWPORT m_viewPort;

	XMFLOAT4X4 m_lightView;
	XMFLOAT4X4 m_lightProj;
	XMFLOAT4X4 m_shadowTransform;

public:
	ShadowMap(UINT width, UINT height);
	~ShadowMap();

	ID3D11ShaderResourceView* GetDepthSRV() { return m_depthSRV; }
	void RenderShadowMap();
	XMMATRIX GetViewProjMatrix();
	XMMATRIX GetShadowMapTransform();
};

#endif // !__SHADOW_MAP__
