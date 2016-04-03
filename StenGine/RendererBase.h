#ifndef __RENDERERBASE__
#define __RENDERERBASE__

#include <Windows.h>
#include <DirectXMath.h>
#include <functional>

#include "DrawCmd.h"

namespace Vertex {
	struct StdMeshVertex {
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangent;
		DirectX::XMFLOAT2 TexUV;
	};

	struct ShadowMapVertex {
		DirectX::XMFLOAT3 Pos;
	};

	struct DebugLine {
		DirectX::XMFLOAT3 Pos;
		//XMFLOAT4 Color;
	};

	struct SkinnedMeshVertex {
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangent;
		DirectX::XMFLOAT2 TexUV;
		DirectX::XMFLOAT4 JointWeights;
		DirectX::XMUINT4  JointIndices;
	};

	struct TerrainVertex {
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT2 TexUV;
		DirectX::XMFLOAT2 BoundsY;
	};
}

using CreateWindowCallback = std::function<BOOL(int32_t, int32_t, HINSTANCE, /*int32_t,*/ HWND&)>;

class Renderer {
public:
	static Renderer* Create(HINSTANCE hInstance, HWND hMainWnd);
	static Renderer* Instance() { return _instance; }

	virtual void Release() = 0;
	virtual bool Init(int32_t width, int32_t height, CreateWindowCallback callback) = 0;
	virtual void Draw() = 0;
	virtual float GetAspectRatio() = 0; 
	virtual int GetScreenWidth() = 0; 
	virtual int GetScreenHeight() = 0;
	virtual void DrawGBuffer() = 0;
	virtual void DrawDeferredShading() = 0;
	virtual void DrawBlurSSAOAndCombine() = 0;
	virtual void DrawDebug() = 0;
	virtual void DrawGodRay() = 0;
	virtual class Skybox* GetSkyBox() = 0;
	virtual void* GetDevice() = 0;
	virtual void* GetDeviceContext() = 0;
	virtual void* GetDepthRS() = 0;
	virtual void UpdateTitle(const char*) = 0;
	virtual void AddDrawCmd(DrawCmd &cmd) = 0;

protected:
	static Renderer* _instance;
};


#endif