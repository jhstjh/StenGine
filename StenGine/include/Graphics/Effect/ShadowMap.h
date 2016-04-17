#ifndef __SHADOW_MAP__
#define __SHADOW_MAP__

#include "Graphics/D3DIncludes.h"
#include "Graphics/Abstraction/RenderTarget.h"

#if GRAPHICS_OPENGL
#include "glew.h"
#endif
namespace StenGine
{

class ShadowMap {
private:
	UINT m_width;
	UINT m_height;
#if GRAPHICS_D3D11
	ID3D11ShaderResourceView* m_depthSRV;
	ID3D11DepthStencilView* m_depthDSV;
	D3D11_VIEWPORT m_viewPort;
#else
	GLuint m_depthTex;
	GLuint m_shadowBuffer;
	uint64_t m_depthTexHandle;
#endif
	XMFLOAT4X4 m_lightView;
	XMFLOAT4X4 m_lightProj;
	XMFLOAT4X4 m_shadowTransform;

	RenderTarget m_shadowTarget;

public:
	ShadowMap(UINT width, UINT height);
	~ShadowMap();
#if GRAPHICS_D3D11
	ID3D11ShaderResourceView* GetDepthSRV() { return m_depthSRV; }
#else
	GLuint GetDepthTex() { return m_depthTex; }
	uint64_t GetDepthTexHandle() { return m_depthTexHandle; }
#endif
	void UpdateShadowMatrix();
	XMMATRIX GetViewMatrix();
	XMMATRIX GetViewProjMatrix();
	XMMATRIX GetShadowMapTransform();

	RenderTarget GetRenderTarget();
	void GetDimension(uint32_t &width, uint32_t &height) { width = m_width, height = m_height; }
};

}
#endif // !__SHADOW_MAP__
