#ifndef __SHADOW_MAP__
#define __SHADOW_MAP__

#include "Graphics/Abstraction/RenderTarget.h"
#include "Graphics/Abstraction/Texture.h"

#include "glew.h"

namespace StenGine
{

class ShadowMap {
private:
	uint32_t m_width;
	uint32_t m_height;

	// TODO unify D3D/GL member 
	ID3D11ShaderResourceView* m_depthSRV;
	ID3D11DepthStencilView* m_depthDSV;

	GLuint m_depthTex;
	GLuint m_shadowBuffer;
	uint64_t m_depthTexHandle;


	Mat4 m_lightView;
	Mat4 m_lightProj;
	Mat4 m_shadowTransform;

	RenderTarget m_shadowTarget;

public:
	ShadowMap(uint32_t width, uint32_t height);
	~ShadowMap();

	ID3D11ShaderResourceView* GetDepthSRV() { return m_depthSRV; }
	GLuint GetDepthTex() { return m_depthTex; }
	uint64_t GetDepthTexHandle() { return m_depthTexHandle; }

	void UpdateShadowMatrix();
	Mat4 GetViewMatrix() const;
	Mat4 GetViewProjMatrix() const;
	Mat4 GetShadowMapTransform() const;

	RenderTarget &GetRenderTarget();
	void GetDimension(uint32_t &width, uint32_t &height) { width = m_width, height = m_height; }
};

}
#endif // !__SHADOW_MAP__
