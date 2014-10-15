#include "CameraManager.h"

Camera::Camera(float px, float py, float pz,
	float tx, float ty, float tz,
	float ux, float uy, float uz,
	float fov, float np, float fp)
{
	XMVECTOR pos = XMVectorSet(px, py, pz, 1.0f);
	XMVECTOR target = XMVectorSet(tx, ty, tz, 0.0f);
	XMVECTOR up = XMVectorSet(ux, uy, uz, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_view, V);

	XMMATRIX P = XMMatrixPerspectiveFovLH(fov, D3D11Renderer::Instance()->GetAspectRatio(), np, fp);
	XMStoreFloat4x4(&m_proj, P);
}

XMMATRIX Camera::GetViewProjMatrix() {
	return XMLoadFloat4x4(&m_view) * XMLoadFloat4x4(&m_proj);
}


CameraManager* CameraManager::_instance = nullptr;

CameraManager::CameraManager() {
	m_debugCamera = new Camera(2.0f, 3.5f, -3.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.25f * 3.14159f, 1.0f, 1000.0f);
	m_activeCamera = m_debugCamera;
}

CameraManager::~CameraManager(){}