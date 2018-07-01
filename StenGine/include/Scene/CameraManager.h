#ifndef __CAMERA_MANAGER__
#define __CAMERA_MANAGER__

#include "Math/MathDefs.h"
#include "Scene/Component.h"
#include "System/AlignedClass.h"
#include "System/API/PlatformAPIDefs.h"
#include "System/SingletonClass.h"

namespace StenGine
{

class Camera : public Component, public AlignedClass<16> 
{
	Mat4 mProj;

	float mNP;
	float mFP;
	float mFOV;

	bool mEnabled{ false };

	void UpdateProjMat();

public:
	Camera(float fov, float np, float fp);

	~Camera();
	Mat4 GetViewProjMatrix();
	Mat4 GetViewMatrix();
	Mat4 GetProjMatrix();
	Vec4 GetPos();

	void SetEnabled(bool enabled);
	void Update();

	void DrawMenu() override;
};

class CameraManager : public SingletonClass<CameraManager>
{
private:
	Camera * mActiveCamera{ nullptr };

public:

	CameraManager();
	~CameraManager();

	void SetActiveCamera(Camera* cam);
	Camera* GetActiveCamera() { return mActiveCamera; }

};

}

#endif // !__CAMERA_MANAGER__
