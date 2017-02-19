#ifndef __CAMERA_MANAGER__
#define __CAMERA_MANAGER__

#include "Math/MathDefs.h"
#include "System/AlignedClass.h"
#include "System/API/PlatformAPIDefs.h"
#include "System/SingletonClass.h"

namespace StenGine
{

class Camera : public AlignedClass<16> {
	Vec3 m_pos;
	Mat4 m_view;
	Mat4 m_proj;
	Mat4 m_worldTransform;
	Vec3 m_target;
	Vec3 m_up;

	float m_radius;
	float m_phi;
	float m_theta;

public:
	Camera(float px, float py, float pz,
		   float tx, float ty, float tz,
		   float ux, float uy, float uz,
		   float fov, float np, float fp);
	//Camera(XMFLOAT4X4 worldTransform;)
	~Camera();
	Mat4 GetViewProjMatrix();
	Mat4 GetViewMatrix();
	Mat4 GetProjMatrix();
	Vec4 GetPos() {
		return{ m_worldTransform(4, 1), m_worldTransform(4, 2), m_worldTransform(4, 2), 0.0 };
	}

	void Update();
};

class CameraManager : public SingletonClass<CameraManager> {
private:
	Camera* m_debugCamera;
	Camera* m_activeCamera;

public:

	CameraManager();
	~CameraManager();

	Camera* GetActiveCamera() { return m_activeCamera; }

};

}

#endif // !__CAMERA_MANAGER__
