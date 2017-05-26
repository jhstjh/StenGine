#ifndef __RENDERERBASE__
#define __RENDERERBASE__

#include <functional>

#include "DrawCmd.h"
#include "Graphics/Abstraction/GPUBuffer.h"
#include "Graphics/Abstraction/RenderTarget.h"
#include "Graphics/Abstraction/Texture.h"
#include "Graphics/Abstraction/UAVBinding.h"
#include "Math/MathDefs.h"
#include "Utility/Semaphore.h"

namespace StenGine
{

enum class RenderBackend
{
	OPENGL4,
	D3D11,
};

namespace Vertex {
struct StdMeshVertex {
	Vec3Packed Pos;
	Vec3Packed Normal;
	Vec3Packed Tangent;
	Vec2Packed TexUV;
};

struct InstanceVertex
{
	Vec3Packed position;
};

struct ShadowMapVertex {
	Vec3Packed Pos;
};

struct DebugLine {
	Vec3Packed Pos;
};

struct SkinnedMeshVertex {
	Vec3Packed Pos;
	Vec3Packed Normal;
	Vec3Packed Tangent;
	Vec2Packed TexUV;
	Vec4Packed JointWeights;
	Vec4uiPacked JointIndices;
};

struct TerrainVertex {
	Vec3Packed Pos;
	Vec2Packed TexUV;
	Vec2Packed BoundsY;
};
}

using CreateWindowCallback = std::function<BOOL(int32_t, int32_t, HINSTANCE, /*int32_t,*/ HWND&)>;
using DrawEventHandler = std::function<void()>;

class Renderer {
public:
	static std::unique_ptr<Renderer> Create(HINSTANCE hInstance, HWND hMainWnd, Semaphore &prepareDrawListSync, Semaphore &finishedDrawListSync);
	static Renderer* Instance() { return _instance; }
	static void SetRenderBackend(RenderBackend backend) { _backend = backend; }

	static RenderBackend GetRenderBackend()
	{
		return _backend;
	}

	virtual ~Renderer() = default;

	virtual void Release() = 0;
	virtual bool Init(int32_t width, int32_t height, CreateWindowCallback callback) = 0;
	virtual void Draw() = 0;
	virtual float GetAspectRatio() = 0;
	virtual int GetScreenWidth() = 0;
	virtual int GetScreenHeight() = 0;
	virtual void DrawShadowMap() = 0;
	virtual void DrawGBuffer() = 0;
	virtual void DrawDeferredShading() = 0;
	virtual void DrawBlurSSAOAndCombine() = 0;
	virtual void DrawDebug() = 0;
	virtual class Skybox* GetSkyBox() = 0;
	virtual void* GetDevice() = 0;
	virtual void* GetDeviceContext() = 0;
	virtual void* GetDepthRS() = 0;
	virtual void UpdateTitle(const char*) = 0;
	virtual void AddDeferredDrawCmd(DrawCmd &cmd) = 0;
	virtual RenderTarget &GetGbuffer() = 0;
	virtual void AddDraw(DrawEventHandler handler) = 0;
	virtual void AddShadowDraw(DrawEventHandler handler) = 0;
	virtual void EndFrame() = 0;
	virtual void AcquireContext() = 0;
	virtual void ReleaseContext() = 0;

	virtual ConstantBuffer CreateConstantBuffer(uint32_t index, uint32_t size, GPUBuffer buffer) = 0;
	virtual GPUBuffer CreateGPUBuffer(size_t size, BufferUsage usage, void* data = nullptr, BufferType type = BufferType::GENERAL) = 0;
	virtual RenderTarget CreateRenderTarget() = 0;
	virtual UAVBinding CreateUAVBinding() = 0;
	virtual Texture CreateTexture(uint32_t width, uint32_t height, void* srv) = 0;

protected:
	static Renderer* _instance;
	static RenderBackend _backend;
};

}
#endif