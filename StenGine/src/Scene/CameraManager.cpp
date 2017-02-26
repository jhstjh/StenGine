#include "Engine/EventSystem.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Input/InputManager.h"
#include "Math/MathHelper.h"
#include "Scene/CameraManager.h"

#define MOVE_SPEED 10

namespace StenGine
{

Camera::Camera(float px, float py, float pz,
	float tx, float ty, float tz,
	float ux, float uy, float uz,
	float fov, float np, float fp)
{
	mTrans = Mat4::FromTranslationVector(Vec3( px, py, pz ));

	mView = Mat4::LookAt({ tx, ty, tz }, { px, py, pz }, { ux, uy, uz }, -1.0f);
	mProj = Mat4::Perspective(fov, Renderer::Instance()->GetAspectRatio(), np, fp, -1.0f);

	mWorldTransform = mView.Inverse();
	mRot = (mTrans.Inverse() * mWorldTransform);
	auto rot3 = Mat3(mRot.GetColumn(0).xyz(), mRot.GetColumn(1).xyz(), mRot.GetColumn(2).xyz());
	mRotQuat = Quat::FromMatrix(rot3);

	if (Renderer::GetRenderBackend() == RenderBackend::OPENGL4)
	{
		mProj(2, 2) = (np + fp) / (fp - np);
		mProj(2, 3) = -2 * (np * fp) / (fp - np);
	}
}

Camera::~Camera()
{

}

Mat4 Camera::GetViewProjMatrix()
{
	return mProj * mView;
}

Mat4 Camera::GetViewMatrix()
{
	return mView;
}

Mat4 Camera::GetProjMatrix()
{
	return mProj;
}

void Camera::Update() 
{
	if (InputManager::Instance()->GetKeyHold('W')) 
	{
		MatrixHelper::MoveForward(mWorldTransform, MOVE_SPEED * Timer::GetDeltaTime());
		mTrans = mWorldTransform * mRot.Inverse();
		mView = mWorldTransform.Inverse();
	}
	if (InputManager::Instance()->GetKeyHold('S')) 
	{
		MatrixHelper::MoveBack(mWorldTransform, MOVE_SPEED * Timer::GetDeltaTime());
		mTrans = mWorldTransform * mRot.Inverse();
		mView = mWorldTransform.Inverse();
	}
	if (InputManager::Instance()->GetKeyHold('A'))
	{
		MatrixHelper::MoveLeft(mWorldTransform, MOVE_SPEED * Timer::GetDeltaTime());
		mTrans = mWorldTransform * mRot.Inverse();
		mView = mWorldTransform.Inverse();
	}
	if (InputManager::Instance()->GetKeyHold('D'))
	{
		MatrixHelper::MoveRight(mWorldTransform, MOVE_SPEED * Timer::GetDeltaTime());
		mTrans = mWorldTransform * mRot.Inverse();
		mView = mWorldTransform.Inverse();
	}
	if (InputManager::Instance()->GetKeyHold(VK_UP))
	{
		auto rot = Quat::FromAngleAxis(-PI / 3.0f * Timer::GetDeltaTime(), { 1, 0, 0 });
		mRotQuat = mRotQuat * rot;
		mRot = mRotQuat.ToMatrix4();

		mWorldTransform = mTrans * mRot;
		mView = mWorldTransform.Inverse();
	}
	if (InputManager::Instance()->GetKeyHold(VK_DOWN))
	{
		auto rot = Quat::FromAngleAxis(PI / 3.0f * Timer::GetDeltaTime(), { 1, 0, 0 });
		mRotQuat = mRotQuat * rot;
		mRot = mRotQuat.ToMatrix4();

		mWorldTransform = mTrans * mRot;
		mView = mWorldTransform.Inverse();
	}
	if (InputManager::Instance()->GetKeyHold(VK_LEFT)) 
	{
		auto rot = Quat::FromAngleAxis(-PI / 3.0f * Timer::GetDeltaTime(), { 0, 1, 0 });
		mRotQuat = rot * mRotQuat;
		mRot = mRotQuat.ToMatrix4();

		mWorldTransform = mTrans * mRot;
		mView = mWorldTransform.Inverse();
	}
	if (InputManager::Instance()->GetKeyHold(VK_RIGHT)) 
	{
		auto rot = Quat::FromAngleAxis(PI / 3.0f * Timer::GetDeltaTime(), { 0, 1, 0 });
		mRotQuat = rot * mRotQuat;
		mRot = mRotQuat.ToMatrix4();

		mWorldTransform = mTrans * mRot;
		mView = mWorldTransform.Inverse();
	}
}

CameraManager::CameraManager()
{
	mDebugCamera = new Camera(4.0f, 11.f, -11.f, 0.0f, 5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.25f * 3.14159f, 1.0f, 1000.0f);
	mActiveCamera = mDebugCamera;

	auto updateCam = [this]()
	{
		mActiveCamera->Update();
	};

	EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::UPDATE, updateCam);
}

CameraManager::~CameraManager() 
{
	mActiveCamera = nullptr;
	SafeDelete(mDebugCamera);
}

DEFINE_SINGLETON_CLASS(CameraManager)

}