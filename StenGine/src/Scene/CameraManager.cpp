#include "stdafx.h"

#include "Engine/EventSystem.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Input/InputManager.h"
#include "Math/MathHelper.h"
#include "Scene/CameraManager.h"
#include "Scene/GameObject.h"

namespace StenGine
{

Camera::Camera(float fov, float np, float fp)
	: mNP(np)
	, mFP(fp)
	, mFOV(fov)
{
	UpdateProjMat();
}

void Camera::UpdateProjMat()
{
	mProj = Mat4::Perspective(mFOV, Renderer::Instance()->GetAspectRatio(), mNP, mFP, -1.0f);

	if (Renderer::GetRenderBackend() == RenderBackend::OPENGL4)
	{
		mProj(2, 2) = (mNP + mFP) / (mFP - mNP);
		mProj(2, 3) = -2 * (mNP * mFP) / (mFP - mNP);
	}
}

Camera::~Camera()
{

}

Mat4 Camera::GetViewProjMatrix()
{
	return mProj * mParent->GetTransform()->GetWorldTransformInversed();
}

Mat4 Camera::GetViewMatrix()
{
	return mParent->GetTransform()->GetWorldTransformInversed();
}

Mat4 Camera::GetProjMatrix()
{
	return mProj;
}

Vec4 Camera::GetPos()
{
	return{ mParent->GetTransform()->GetWorldTransform()(0, 3), 
			mParent->GetTransform()->GetWorldTransform()(1, 3),
			mParent->GetTransform()->GetWorldTransform()(2, 3), 0.0 };
}

void Camera::SetEnabled(bool enabled)
{
	mEnabled = enabled;

	if (mEnabled)
	{
		CameraManager::Instance()->SetActiveCamera(this);
	}

	// TODO clear active cam
}

void Camera::Update() 
{

}

void Camera::DrawMenu()
{
	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool dirty = false;

		if (ImGui::SliderFloat("FOV", &mFOV, 0, PI)) dirty = true;
		if (ImGui::InputFloat("Near Plane", &mNP)) dirty = true;
		if (ImGui::InputFloat("Far Plane", &mFP)) dirty = true;
		
		if (dirty)
		{
			UpdateProjMat();
		}

		if (ImGui::Button("Set Current"))
		{
			CameraManager::Instance()->SetActiveCamera(this);
		}
	}
}


CameraManager::CameraManager()
{
	auto updateCam = [this]()
	{
		mActiveCamera->Update();
	};

	EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::UPDATE, updateCam);
}

CameraManager::~CameraManager() 
{
	mActiveCamera = nullptr;
}

void CameraManager::SetActiveCamera(Camera* cam)
{
	mActiveCamera = cam;
}

DEFINE_SINGLETON_CLASS(CameraManager)

}