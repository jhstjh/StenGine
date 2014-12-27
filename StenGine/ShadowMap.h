#ifndef __SHADOW_MAP__
#define __SHADOW_MAP__

#include "D3DIncludes.h"
#include "GL/glew.h"

class ShadowMap {
private:
	UINT m_width;
	UINT m_height;
#ifdef GRAPHICS_D3D11
	ID3D11ShaderResourceView* m_depthSRV;
	ID3D11DepthStencilView* m_depthDSV;
	D3D11_VIEWPORT m_viewPort;
#else
	GLuint m_depthTex;
	GLuint m_shadowBuffer;
#endif
	XMFLOAT4X4 m_lightView;
	XMFLOAT4X4 m_lightProj;
	XMFLOAT4X4 m_shadowTransform;

public:
	ShadowMap(UINT width, UINT height);
	~ShadowMap();
#ifdef GRAPHICS_D3D11
	ID3D11ShaderResourceView* GetDepthSRV() { return m_depthSRV; }
#else
	GLuint GetDepthTex() { return m_depthTex; }
#endif
	void RenderShadowMap();
	XMMATRIX GetViewProjMatrix();
	XMMATRIX GetShadowMapTransform();
};

#endif // !__SHADOW_MAP__
