#include "ShadowMap.h"
#include "D3D11Renderer.h"
#include "LightManager.h"

ShadowMap::ShadowMap(UINT width, UINT height)
	:m_width(width), m_height(height)
{
#ifdef GRAPHICS_D3D11
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
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateTexture2D(&texDesc, 0, &depthMap));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateDepthStencilView(depthMap, &dsvDesc, &m_depthDSV));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateShaderResourceView(depthMap, &srvDesc, &m_depthSRV));

	ReleaseCOM(depthMap);
#else
	glGenFramebuffers(1, &m_shadowBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowBuffer);

	glGenTextures(1, &m_depthTex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_depthTex);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT,
		m_width,
		m_height,
		0,
		GL_DEPTH_COMPONENT,
		GL_UNSIGNED_BYTE,
		NULL
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// clamp to edge. clamp to border may reduce artifacts outside light frustum
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// attach depth texture to framebuffer
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTex, 0);

	// tell framebuffer not to use any colour drawing outputs
	GLenum draw_bufs[] = { GL_NONE };
	glDrawBuffers(1, draw_bufs);

	// bind default framebuffer again
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

ShadowMap::~ShadowMap() {
#ifdef GRAPHICS_D3D11
	ReleaseCOM(m_depthSRV);
	ReleaseCOM(m_depthDSV);
#endif
}

XMMATRIX ShadowMap::GetViewProjMatrix() {
	return XMLoadFloat4x4(&m_lightView) * XMLoadFloat4x4(&m_lightProj);
}

XMMATRIX ShadowMap::GetShadowMapTransform() {
	return XMLoadFloat4x4(&m_shadowTransform);
}

void ShadowMap::RenderShadowMap() {
	// only build shadow map for first directional light for now

	XMVECTOR lightDir = XMLoadFloat3(&(LightManager::Instance()->m_dirLights[0]->direction));
	XMVECTOR lightPos = -50 * XMLoadFloat3(&(LightManager::Instance()->m_dirLights[0]->direction));
	XMVECTOR lightTarget = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(lightPos, lightTarget, up);

	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(lightTarget, V));

	float l = sphereCenterLS.x - 10;
	float b = sphereCenterLS.y - 10;
	float n = sphereCenterLS.z - 10;
	float r = sphereCenterLS.x + 10;
	float t = sphereCenterLS.y + 10;
	float f = sphereCenterLS.z + 10;
	XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

#ifdef GRAPHICS_D3D11
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

#ifdef GRAPHICS_D3D11

	ID3D11DeviceContext* dc = D3D11Renderer::Instance()->GetD3DContext();

	dc->RSSetViewports(1, &m_viewPort);

	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	dc->OMSetRenderTargets(1, renderTargets, m_depthDSV);
	dc->ClearDepthStencilView(m_depthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	D3D11Renderer::Instance()->GetD3DContext()->RSSetState(D3D11Renderer::Instance()->m_depthRS);
	D3D11Renderer::Instance()->GetD3DContext()->IASetInputLayout(EffectsManager::Instance()->m_shadowMapEffect->GetInputLayout());
	D3D11Renderer::Instance()->GetD3DContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#else
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, m_width, m_height);
#endif
	EffectsManager::Instance()->m_shadowMapEffect->SetShader();
	
	for (int iMesh = 0; iMesh < EffectsManager::Instance()->m_deferredGeometryPassEffect->m_associatedMeshes.size(); iMesh++) {
		if (EffectsManager::Instance()->m_deferredGeometryPassEffect->m_associatedMeshes[iMesh]->m_castShadow)
			EffectsManager::Instance()->m_deferredGeometryPassEffect->m_associatedMeshes[iMesh]->DrawOnShadowMap();
	}

	EffectsManager::Instance()->m_shadowMapEffect->UnSetShader();

#ifdef GRAPHICS_D3D11
	D3D11Renderer::Instance()->GetD3DContext()->RSSetState(0);
#else
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

}