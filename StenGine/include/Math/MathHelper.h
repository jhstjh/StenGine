#ifndef __MATH_HELPER__
#define __MATH_HELPER__

#include <DirectXMath.h>

using namespace DirectX;

#if GRAPHICS_D3D11
#define TRASNPOSE_API_CHOOSER(M) XMMatrixTranspose(M) 
#elif  GRAPHICS_OPENGL
#define TRASNPOSE_API_CHOOSER(M) (M) 
#endif
namespace StenGine
{

static const float PI = 3.1415926f;
static const XMMATRIX IDENTITY_MAT = XMMatrixIdentity();

class MatrixHelper {
public:
	inline static XMFLOAT3 GetPosition(const XMFLOAT4X4& mat) { return XMFLOAT3(mat._41, mat._42, mat._43); }
	inline static void SetPosition(XMFLOAT4X4 &mat, float x, float y, float z) { mat._41 = x; mat._42 = y; mat._43 = z; }
	inline static XMFLOAT3 GetForward(const XMFLOAT4X4& mat) { return XMFLOAT3(mat._31, mat._32, mat._33); }
	inline static XMFLOAT3 GetRight(const XMFLOAT4X4& mat) { return XMFLOAT3(mat._11, mat._12, mat._13); }
	inline static void MoveForward(XMFLOAT4X4& mat, float distance) {
		XMFLOAT3 offset = GetForward(mat);
		mat._41 += offset.x * distance;
		mat._42 += offset.y * distance;
		mat._43 += offset.z * distance;
	}
	inline static void MoveBack(XMFLOAT4X4& mat, float distance) { MoveForward(mat, -distance); }
	inline static void MoveRight(XMFLOAT4X4& mat, float distance) {
		XMFLOAT3 offset = GetRight(mat);
		mat._41 += offset.x * distance;
		mat._42 += offset.y * distance;
		mat._43 += offset.z * distance;
	}
	inline static void MoveLeft(XMFLOAT4X4& mat, float distance) { MoveRight(mat, -distance); }
	inline static XMFLOAT3 NormalizeFloat3(XMFLOAT3& vec3) {
		float len = sqrt(vec3.x * vec3.x + vec3.y * vec3.y + vec3.z * vec3.z);
		vec3.x /= len;
		vec3.y /= len;
		vec3.z /= len;
		return vec3;
	}
	inline static XMMATRIX InverseTranspose(CXMMATRIX M)
	{
		XMMATRIX A = M;
		A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		XMVECTOR det = XMMatrixDeterminant(A);
		return XMMatrixTranspose(XMMatrixInverse(&det, A));
	}
	inline static XMMATRIX Inverse(CXMMATRIX M)
	{
		XMVECTOR det = XMMatrixDeterminant(M);
		return XMMatrixInverse(&det, M);
	}
};

}
#endif // !__MATH_HELPER__
