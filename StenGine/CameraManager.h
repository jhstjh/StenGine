#ifndef __CAMERA_MANAGER__
#define __CAMERA_MANAGER__

#include "D3D11Renderer.h"

class Camera {
	XMFLOAT4 m_pos;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;
	XMVECTOR m_target;
	XMVECTOR m_up;

	float m_radius;
	float m_phi;
	float m_theta;
	POINT m_lastMousePos;

public:
	Camera(float px, float py, float pz,
		   float tx, float ty, float tz,
		   float ux, float uy, float uz,
		   float fov, float np, float fp);
	~Camera();
	XMMATRIX GetViewProjMatrix();
	XMFLOAT4 GetPos() { return m_pos; };
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
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
