#ifndef __CAMERA_MANAGER__
#define __CAMERA_MANAGER__

#include "Math/MathDefs.h"
#include "System/AlignedClass.h"
#include "System/API/PlatformAPIDefs.h"
#include "System/SingletonClass.h"

namespace StenGine
{

class Camera : public AlignedClass<16> 
{
	// TODO Make this a generic GameObject and use Transform !
	Mat4 mTrans;
	Mat4 mRot;
	// Mat4 mScale; // Identity
	Mat4 mView;
	Mat4 mProj;
	Mat4 mWorldTransform;
	Vec3 mRotEuler;

public:
	Camera(float px, float py, float pz,
		   float tx, float ty, float tz,
		   float ux, float uy, float uz,
		   float fov, float np, float fp);

	~Camera();
	Mat4 GetViewProjMatrix();
	Mat4 GetViewMatrix();
	Mat4 GetProjMatrix();
	Vec4 GetPos() 
	{
		return{ mWorldTransform(0, 3), mWorldTransform(1, 3), mWorldTransform(2, 3), 0.0 };
	}

	void Update();
};

class CameraManager : public SingletonClass<CameraManager>
{
private:
	Camera* mDebugCamera;
	Camera* mActiveCamera;

public:

	CameraManager();
	~CameraManager();

	Camera* GetActiveCamera() { return mActiveCamera; }

};

}

#endif // !__CAMERA_MANAGER__
