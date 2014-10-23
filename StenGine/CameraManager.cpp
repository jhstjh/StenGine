#include "CameraManager.h"

Camera::Camera(float px, float py, float pz,
	float tx, float ty, float tz,
	float ux, float uy, float uz,
	float fov, float np, float fp)
{
	XMVECTOR pos = XMVectorSet(px, py, pz, 0.0f);
	m_target = XMVectorSet(tx, ty, tz, 0.0f);
	m_up = XMVectorSet(ux, uy, uz, 0.0f);
	XMStoreFloat4(&m_pos, pos);

	XMMATRIX V = XMMatrixLookAtLH(pos, m_target, m_up);
	XMStoreFloat4x4(&m_view, V);

	XMMATRIX P = XMMatrixPerspectiveFovLH(fov, D3D11Renderer::Instance()->GetAspectRatio(), np, fp);
	XMStoreFloat4x4(&m_proj, P);

	m_radius = XMVectorGetX(XMVector3Length(pos - m_target));
	m_phi = acosf((py - ty) / m_radius);
	m_theta = asinf((pz - tz) / (m_radius * sinf(m_phi)));
}

XMMATRIX Camera::GetViewProjMatrix() {
	return XMLoadFloat4x4(&m_view) * XMLoadFloat4x4(&m_proj);
}

XMMATRIX Camera::GetViewMatrix() {
	return XMLoadFloat4x4(&m_view);
}

XMMATRIX Camera::GetProjMatrix() {
	return XMLoadFloat4x4(&m_proj);
}

void Camera::OnMouseDown(WPARAM btnState, int x, int y) {
	m_lastMousePos.x = x;
	m_lastMousePos.y = y;
}

void Camera::OnMouseUp(WPARAM btnState, int x, int y) {

}

void Camera::OnMouseMove(WPARAM btnState, int x, int y) {
	if ((btnState & MK_LBUTTON)) {
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - m_lastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - m_lastMousePos.y));
		m_theta -= dx;
		m_phi -= dy;

		m_phi = min(max(m_phi, 0.1f), 3.14159f - 0.1f);
		
	}
	m_lastMousePos.x = x;
	m_lastMousePos.y = y;


	float px = m_radius*sinf(m_phi)*cosf(m_theta);
	float pz = m_radius*sinf(m_phi)*sinf(m_theta);
	float py = m_radius*cosf(m_phi);

	XMVECTOR pos = XMVectorSet(px, py, pz, 0.0f);
	XMStoreFloat4(&m_pos, pos);

	XMMATRIX V = XMMatrixLookAtLH(pos, m_target, m_up);
	XMStoreFloat4x4(&m_view, V);
}


CameraManager* CameraManager::_instance = nullptr;

CameraManager::CameraManager() {
	m_debugCamera = new Camera(2.0f, 5.5f, -5.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.25f * 3.14159f, 1.0f, 1000.0f);
	m_activeCamera = m_debugCamera;
}

CameraManager::~CameraManager(){}