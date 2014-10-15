#ifndef __CAMERA_MANAGER__
#define __CAMERA_MANAGER__

#include "D3D11Renderer.h"

class Camera {
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

public:
	Camera(float px, float py, float pz,
		   float tx, float ty, float tz,
		   float ux, float uy, float uz,
		   float fov, float np, float fp);
	~Camera();
	XMMATRIX GetViewProjMatrix();
};

class CameraManager {
private:
	Camera* m_debugCamera;
	Camera* m_activeCamera;
	static CameraManager* _instance;

public:
	static CameraManager* Instance() {
		if (!_instance)
			_instance = new CameraManager();
		return _instance;
	}

	CameraManager();
	~CameraManager();

	Camera* GetActiveCamera() { return m_activeCamera; }

};




#endif // !__CAMERA_MANAGER__
