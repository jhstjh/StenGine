#include "Scene/CameraManager.h"

#if PLATFORM_WIN32
#include "Input/InputManager.h"
#include "Graphics/Abstraction/RendererBase.h"
using namespace DirectX;
#elif  PLATFORM_ANDROID
#include "GLESRenderer.h"
#endif

#include "Math/MathHelper.h"

#define MOVE_SPEED 10

namespace StenGine
{

Camera::Camera(float px, float py, float pz,
	float tx, float ty, float tz,
	float ux, float uy, float uz,
	float fov, float np, float fp)
{
#if PLATFORM_WIN32

	XMVECTOR pos = XMVectorSet(px, py, pz, 0.0f);
	m_target = XMVectorSet(tx, ty, tz, 0.0f);
	m_up = XMVectorSet(ux, uy, uz, 0.0f);
	XMStoreFloat4(&m_pos, pos);

	XMMATRIX V = XMMatrixLookAtLH(pos, m_target, m_up);
	XMStoreFloat4x4(&m_view, V);
#if GRAPHICS_D3D11
	XMMATRIX P = XMMatrixPerspectiveFovLH(fov, Renderer::Instance()->GetAspectRatio(), np, fp);
#else
	XMMATRIX P = XMMatrixPerspectiveFovLH(fov, Renderer::Instance()->GetAspectRatio(), np, fp);
#endif
	XMStoreFloat4x4(&m_proj, P);

#if GRAPHICS_OPENGL
	m_proj.m[2][2] = (np + fp) / (fp - np);
	m_proj.m[3][2] = - 2 * (np * fp) / (fp - np);
#endif

	m_radius = XMVectorGetX(XMVector3Length(pos - m_target));
	m_phi = acosf((py - ty) / m_radius);
	m_theta = asinf((pz - tz) / (m_radius * sinf(m_phi)));

	XMStoreFloat4x4(&m_worldTransform, MatrixHelper::Inverse(V));
#elif  PLATFORM_ANDROID

// 	m_target = ndk_helper::Vec4(tx, ty, tz, 0.0f);
// 	m_up = XMVectorSet(ux, uy, uz, 0.0f);
// 	XMStoreFloat4(&m_pos, pos);

	m_view = ndk_helper::Mat4::LookAt(ndk_helper::Vec3(px, py, pz),
		ndk_helper::Vec3(tx, ty, tz), ndk_helper::Vec3(ux, uy, uz));

	int32_t viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	float fAspect = (float)viewport[2] / (float)viewport[3];

	m_proj = ndk_helper::Mat4::Perspective(fAspect, 1.f, np, fp);

#endif
}

Camera::~Camera()
{

}

XMMATRIX Camera::GetViewProjMatrix() {
#if PLATFORM_WIN32
	return XMLoadFloat4x4(&m_view) * XMLoadFloat4x4(&m_proj);
#elif  PLATFORM_ANDROID
	return m_proj * m_view;
#endif
}

XMMATRIX Camera::GetViewMatrix() {
#if PLATFORM_WIN32
	return XMLoadFloat4x4(&m_view);
#elif  PLATFORM_ANDROID
	return m_view;
#endif
}

XMMATRIX Camera::GetProjMatrix() {
#if PLATFORM_WIN32
	return XMLoadFloat4x4(&m_proj);
#elif  PLATFORM_ANDROID
	return m_proj;
#endif
}

// void Camera::OnMouseDown(WPARAM btnState, int x, int y) {
// 	m_lastMousePos.x = x;
// 	m_lastMousePos.y = y;
// }
// 
// void Camera::OnMouseUp(WPARAM btnState, int x, int y) {
// 
// }
// 
// void Camera::OnMouseMove(WPARAM btnState, int x, int y) {
// 	if ((btnState & MK_LBUTTON)) {
// 		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - m_lastMousePos.x));
// 		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - m_lastMousePos.y));
// 		m_theta -= dx;
// 		m_phi -= dy;
// 
// 		m_phi = min(max(m_phi, 0.1f), 3.14159f - 0.1f);
// 		
// 	}
// 	m_lastMousePos.x = x;
// 	m_lastMousePos.y = y;
// 
// 
// 	float px = m_radius*sinf(m_phi)*cosf(m_theta);
// 	float pz = m_radius*sinf(m_phi)*sinf(m_theta);
// 	float py = m_radius*cosf(m_phi);
// 
// 	XMVECTOR pos = XMVectorSet(px, py, pz, 0.0f);
// 	XMStoreFloat4(&m_pos, pos);
// 
// 	XMMATRIX V = XMMatrixLookAtLH(pos, m_target, m_up);
// 	XMStoreFloat4x4(&m_view, V);
//}

void Camera::Update() {
#if PLATFORM_WIN32
	if (InputManager::Instance()->GetKeyHold('W')) {
		MatrixHelper::MoveForward(m_worldTransform, MOVE_SPEED * Timer::GetDeltaTime());
		XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(XMLoadFloat4x4(&m_worldTransform)));
	}
	if (InputManager::Instance()->GetKeyHold('S')) {
		MatrixHelper::MoveBack(m_worldTransform, MOVE_SPEED * Timer::GetDeltaTime());
		XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(XMLoadFloat4x4(&m_worldTransform)));
	}
	if (InputManager::Instance()->GetKeyHold('A')) {
		MatrixHelper::MoveLeft(m_worldTransform, MOVE_SPEED * Timer::GetDeltaTime());
		XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(XMLoadFloat4x4(&m_worldTransform)));
	}
	if (InputManager::Instance()->GetKeyHold('D')) {
		MatrixHelper::MoveRight(m_worldTransform, MOVE_SPEED * Timer::GetDeltaTime());
		XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(XMLoadFloat4x4(&m_worldTransform)));
	}
	if (InputManager::Instance()->GetKeyHold(VK_UP)) {
		XMMATRIX M = XMMatrixRotationX(-3.14159f / 3.0f * Timer::GetDeltaTime()) * XMLoadFloat4x4(&m_worldTransform);
		XMStoreFloat4x4(&m_worldTransform, M);
		XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(M));
	}
	if (InputManager::Instance()->GetKeyHold(VK_DOWN)) {
		XMMATRIX M = XMMatrixRotationX(3.14159f / 3.0f * Timer::GetDeltaTime()) * XMLoadFloat4x4(&m_worldTransform);
		XMStoreFloat4x4(&m_worldTransform, M);
		XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(M));
	}
	if (InputManager::Instance()->GetKeyHold(VK_LEFT)) {
		XMFLOAT4 pos = GetPos();
		MatrixHelper::SetPosition(m_worldTransform, 0, 0, 0);
		XMMATRIX M = XMLoadFloat4x4(&m_worldTransform) * XMMatrixRotationY(-3.14159f / 3.0f * Timer::GetDeltaTime());
		M.r[3] = XMVectorSet(pos.x, pos.y, pos.z, 1.0);
		XMStoreFloat4x4(&m_worldTransform, M);
		XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(M));
	}
	if (InputManager::Instance()->GetKeyHold(VK_RIGHT)) {
		XMFLOAT4 pos = GetPos();
		MatrixHelper::SetPosition(m_worldTransform, 0, 0, 0);
		XMMATRIX M = XMLoadFloat4x4(&m_worldTransform) * XMMatrixRotationY(3.14159f / 3.0f * Timer::GetDeltaTime());
		M.r[3] = XMVectorSet(pos.x, pos.y, pos.z, 1.0);
		XMStoreFloat4x4(&m_worldTransform, M);
		XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(M));
	}
#endif
}

CameraManager* CameraManager::_instance = nullptr;

CameraManager::CameraManager() {
	m_debugCamera = new Camera(4.0f, 11.f, -11.f, 0.0f, 5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.25f * 3.14159f, 1.0f, 1000.0f);
	m_activeCamera = m_debugCamera;
}

CameraManager::~CameraManager() {
	SafeDelete(m_debugCamera);
}

}