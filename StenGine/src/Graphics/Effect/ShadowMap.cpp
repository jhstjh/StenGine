#include "Graphics/Effect/ShadowMap.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Scene/LightManager.h"
#include "Graphics/Abstraction/RendererBase.h"

#include "Mesh/Terrain.h"

#pragma warning(disable: 4312) // 'type cast': conversion from 'GLuint' to 'void *' of greater size

namespace StenGine
{

ShadowMap::ShadowMap(uint32_t width, uint32_t height)
	:m_width(width), m_height(height)
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = m_width;
		texDesc.Height = m_height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		ID3D11Texture2D* depthMap;
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateTexture2D(&texDesc, 0, &depthMap));

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = 0;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateDepthStencilView(depthMap, &dsvDesc, &m_depthDSV));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateShaderResourceView(depthMap, &srvDesc, &m_depthSRV));

		ReleaseCOM(depthMap);

		m_shadowTarget = Renderer::Instance()->CreateRenderTarget();
		m_shadowTarget->AddRenderTarget(nullptr);
		m_shadowTarget->AssignDepthStencil(m_depthDSV);

		break;
	}
	case RenderBackend::OPENGL4:
	{
		GLuint shadowFB;
		glCreateFramebuffers(1, &shadowFB);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_depthTex);
		glTextureStorage2D(m_depthTex, 1, GL_DEPTH_COMPONENT32, width, height);

		glTextureParameteri(m_depthTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_depthTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_depthTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTextureParameteri(m_depthTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float white[] = { 1.0, 1.0, 1.0, 1.0 };
		glTextureParameterfv(m_depthTex, GL_TEXTURE_BORDER_COLOR, white);

		// attach depth texture to framebuffer
		glNamedFramebufferTexture(
			shadowFB, GL_DEPTH_ATTACHMENT, m_depthTex, 0);

		// tell framebuffer not to use any colour drawing outputs
		GLenum draw_bufs[] = { GL_NONE };
		glNamedFramebufferDrawBuffers(shadowFB, 1, draw_bufs);

		m_shadowTarget = Renderer::Instance()->CreateRenderTarget();
		m_shadowTarget->Set(shadowFB);

		m_depthTexHandle = glGetTextureHandleARB(m_depthTex);
		glMakeTextureHandleResidentARB(m_depthTexHandle);

		break;
	}
	}
}

ShadowMap::~ShadowMap() {
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		ReleaseCOM(m_depthSRV);
		ReleaseCOM(m_depthDSV);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		glMakeTextureHandleNonResidentARB(m_depthTexHandle);
		glDeleteTextures(1, &m_depthTex);
		break;
	}
	}
}

Mat4 ShadowMap::GetViewMatrix() const
{
	return m_lightView;
}

Mat4 ShadowMap::GetViewProjMatrix() const
{
	return m_lightProj * m_lightView;
}

Mat4 ShadowMap::GetShadowMapTransform() const
{
	return m_shadowTransform;
}

RenderTarget &ShadowMap::GetRenderTarget()
{
	return m_shadowTarget;
}

void ShadowMap::UpdateShadowMatrix() {
	// only build shadow map for first directional light for now
	auto dir = LightManager::Instance()->m_dirLights[0]->direction;

	Vec3 lightDir{ dir.data };
	Vec3 lightPos = lightDir * -100;
	Vec3 lightTarget{ 0, 0, 0 };
	Vec3 up{ 0, 1, 0 };

	m_lightView = Mat4::LookAt(lightTarget, lightPos, up, -1.f);

	Vec3 sphereCenterLS;
	sphereCenterLS = m_lightView * lightTarget;

	float l = sphereCenterLS.x() - 50;
	float b = sphereCenterLS.y() - 50;
	float n = sphereCenterLS.z() - 50;
	float r = sphereCenterLS.x() + 50;
	float t = sphereCenterLS.y() + 50;
	float f = sphereCenterLS.z() + 50;
	m_lightProj = Mat4::Ortho(l, r, b, t, n, f, -1.f);

	float yScale = -0.5;
	float zScale = 1.0;
	float zTrans = 0.0;

	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		yScale = -0.5;
		zScale = 1.0;
		zTrans = 0.0;
		break;
	}
	case RenderBackend::OPENGL4:
	{
		yScale = 0.5;
		zScale = 0.5;
		zTrans = 0.5;
		break;
	}
	}

	Mat4 T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, yScale, 0.0f, 0.0f,
		0.0f, 0.0f, zScale, 0.0f,
		0.5f, 0.5f, zTrans, 1.0f);

	m_shadowTransform = T * m_lightProj * m_lightView;
}

}