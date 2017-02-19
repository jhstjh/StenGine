#include "Scene/CameraManager.h"
#include "Input/InputManager.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Engine/EventSystem.h"

#include "Math/MathHelper.h"

#define MOVE_SPEED 10

namespace StenGine
{

Camera::Camera(float px, float py, float pz,
	float tx, float ty, float tz,
	float ux, float uy, float uz,
	float fov, float np, float fp)
{
	m_pos = { px, py, pz };
	m_target = { tx, ty, tz };
	m_up = { ux, uy, uz };

	m_view = Mat4::LookAt(m_target, m_pos, m_up, -1.0f);
	m_proj = Mat4::Perspective(fov, Renderer::Instance()->GetAspectRatio(), np, fp, -1.0f);

	// if (Renderer::GetRenderBackend() == RenderBackend::OPENGL4)
	// {
	// 	m_proj.m[2][2] = (np + fp) / (fp - np);
	// 	m_proj.m[3][2] = -2 * (np * fp) / (fp - np);
	// }

	m_radius = (m_pos - m_target).Length();
	m_phi = acosf((py - ty) / m_radius);
	m_theta = asinf((pz - tz) / (m_radius * sinf(m_phi)));

	m_worldTransform = m_view.Inverse();
}

Camera::~Camera()
{

}

Mat4 Camera::GetViewProjMatrix()
{
	return m_proj * m_view;
}

Mat4 Camera::GetViewMatrix()
{
	return m_view;
}

Mat4 Camera::GetProjMatrix()
{
	return m_proj;
}

void Camera::Update() 
{
	// if (InputManager::Instance()->GetKeyHold('W')) {
	// 	MatrixHelper::MoveForward(m_worldTransform, MOVE_SPEED * Timer::GetDeltaTime());
	// 	XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(XMLoadFloat4x4(&m_worldTransform)));
	// }
	// if (InputManager::Instance()->GetKeyHold('S')) {
	// 	MatrixHelper::MoveBack(m_worldTransform, MOVE_SPEED * Timer::GetDeltaTime());
	// 	XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(XMLoadFloat4x4(&m_worldTransform)));
	// }
	// if (InputManager::Instance()->GetKeyHold('A')) {
	// 	MatrixHelper::MoveLeft(m_worldTransform, MOVE_SPEED * Timer::GetDeltaTime());
	// 	XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(XMLoadFloat4x4(&m_worldTransform)));
	// }
	// if (InputManager::Instance()->GetKeyHold('D')) {
	// 	MatrixHelper::MoveRight(m_worldTransform, MOVE_SPEED * Timer::GetDeltaTime());
	// 	XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(XMLoadFloat4x4(&m_worldTransform)));
	// }
	// if (InputManager::Instance()->GetKeyHold(VK_UP)) {
	// 	XMMATRIX M = XMMatrixRotationX(-3.14159f / 3.0f * Timer::GetDeltaTime()) * XMLoadFloat4x4(&m_worldTransform);
	// 	XMStoreFloat4x4(&m_worldTransform, M);
	// 	XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(M));
	// }
	// if (InputManager::Instance()->GetKeyHold(VK_DOWN)) {
	// 	XMMATRIX M = XMMatrixRotationX(3.14159f / 3.0f * Timer::GetDeltaTime()) * XMLoadFloat4x4(&m_worldTransform);
	// 	XMStoreFloat4x4(&m_worldTransform, M);
	// 	XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(M));
	// }
	// if (InputManager::Instance()->GetKeyHold(VK_LEFT)) {
	// 	XMFLOAT4 pos = GetPos();
	// 	MatrixHelper::SetPosition(m_worldTransform, 0, 0, 0);
	// 	XMMATRIX M = XMLoadFloat4x4(&m_worldTransform) * XMMatrixRotationY(-3.14159f / 3.0f * Timer::GetDeltaTime());
	// 	M.r[3] = XMVectorSet(pos.x, pos.y, pos.z, 1.0);
	// 	XMStoreFloat4x4(&m_worldTransform, M);
	// 	XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(M));
	// }
	// if (InputManager::Instance()->GetKeyHold(VK_RIGHT)) {
	// 	XMFLOAT4 pos = GetPos();
	// 	MatrixHelper::SetPosition(m_worldTransform, 0, 0, 0);
	// 	XMMATRIX M = XMLoadFloat4x4(&m_worldTransform) * XMMatrixRotationY(3.14159f / 3.0f * Timer::GetDeltaTime());
	// 	M.r[3] = XMVectorSet(pos.x, pos.y, pos.z, 1.0);
	// 	XMStoreFloat4x4(&m_worldTransform, M);
	// 	XMStoreFloat4x4(&m_view, MatrixHelper::Inverse(M));
	// }
}

CameraManager::CameraManager()
{
	m_debugCamera = new Camera(4.0f, 11.f, -11.f, 0.0f, 5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.25f * 3.14159f, 1.0f, 1000.0f);
	m_activeCamera = m_debugCamera;

	auto updateCam = [this]()
	{
		m_activeCamera->Update();
	};

	EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::UPDATE, updateCam);
}

CameraManager::~CameraManager() 
{
	SafeDelete(m_debugCamera);
}

DEFINE_SINGLETON_CLASS(CameraManager)

}