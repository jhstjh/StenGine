#include "Graphics/Effect/ShadowMap.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Scene/LightManager.h"

#include "Mesh/Terrain.h"

#pragma warning(disable: 4312) // 'type cast': conversion from 'GLuint' to 'void *' of greater size

namespace StenGine
{

ShadowMap::ShadowMap(UINT width, UINT height)
	:m_width(width), m_height(height)
{
#if GRAPHICS_D3D11
	m_viewPort.TopLeftX = 0;
	m_viewPort.TopLeftY = 0;
	m_viewPort.Height = static_cast<float>(height);
	m_viewPort.Width = static_cast<float>(width);
	m_viewPort.MaxDepth = 1;
	m_viewPort.MinDepth = 0;

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

	m_shadowTarget.rtvs.push_back(nullptr);
	m_shadowTarget.dsv = m_depthDSV;

#else
	glCreateFramebuffers(1, &m_shadowTarget);

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
		m_shadowTarget, GL_DEPTH_ATTACHMENT, m_depthTex, 0);

	// tell framebuffer not to use any colour drawing outputs
	GLenum draw_bufs[] = { GL_NONE };
	glNamedFramebufferDrawBuffers(m_shadowTarget, 1, draw_bufs);

	m_depthTexHandle = glGetTextureHandleARB(m_depthTex);
	glMakeTextureHandleResidentARB(m_depthTexHandle);
#endif
}

ShadowMap::~ShadowMap() {
#if GRAPHICS_D3D11
	ReleaseCOM(m_depthSRV);
	ReleaseCOM(m_depthDSV);
#else
	glMakeTextureHandleNonResidentARB(m_depthTexHandle);
#endif
}

XMMATRIX ShadowMap::GetViewMatrix() {
	return XMLoadFloat4x4(&m_lightView);
}

XMMATRIX ShadowMap::GetViewProjMatrix() {
	return XMLoadFloat4x4(&m_lightView) * XMLoadFloat4x4(&m_lightProj);
}

XMMATRIX ShadowMap::GetShadowMapTransform() {
	return XMLoadFloat4x4(&m_shadowTransform);
}

RenderTarget ShadowMap::GetRenderTarget()
{
	return m_shadowTarget;
}

void ShadowMap::UpdateShadowMatrix() {
	// only build shadow map for first directional light for now

	XMVECTOR lightDir = XMLoadFloat3(&(LightManager::Instance()->m_dirLights[0]->direction));
	XMVECTOR lightPos = -100 * XMLoadFloat3(&(LightManager::Instance()->m_dirLights[0]->direction));
	XMVECTOR lightTarget = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(lightPos, lightTarget, up);

	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(lightTarget, V));

	float l = sphereCenterLS.x - 50;
	float b = sphereCenterLS.y - 50;
	float n = sphereCenterLS.z - 50;
	float r = sphereCenterLS.x + 50;
	float t = sphereCenterLS.y + 50;
	float f = sphereCenterLS.z + 50;
	XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

#if GRAPHICS_D3D11
	float yScale = -0.5;
	float zScale = 1.0;
	float zTrans = 0.0;
#else
	float yScale = 0.5;
	float zScale = 0.5;
	float zTrans = 0.5;
#endif

	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, yScale, 0.0f, 0.0f,
		0.0f, 0.0f, zScale, 0.0f,
		0.5f, 0.5f, zTrans, 1.0f);

	XMMATRIX S = V*P*T;

	XMStoreFloat4x4(&m_lightView, V);
	XMStoreFloat4x4(&m_lightProj, P);
	XMStoreFloat4x4(&m_shadowTransform, S);
}

}