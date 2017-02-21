#ifndef __MATH_HELPER__
#define __MATH_HELPER__

#include <DirectXMath.h>
#include "Graphics/Abstraction/RendererBase.h"

using namespace DirectX;

namespace StenGine
{

inline XMMATRIX TRASNPOSE_API_CHOOSER(const XMMATRIX& mat)
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
		return XMMatrixTranspose(mat);
	case RenderBackend::OPENGL4:
		return mat;
	}
	return mat;
}

inline Mat4 TRASNPOSE_API_CHOOSER(const Mat4& mat)
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
		return mat.Transpose();
	case RenderBackend::OPENGL4:
		return mat;
	}
	return mat;
}

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


	inline static void SetPosition(Mat4 &mat, float x, float y, float z) { mat(0, 3) = x; mat(1, 3) = y; mat(2, 3) = z; }

	inline static void MoveLeft(Mat4& mat, float distance) { MoveRight(mat, -distance); }

	inline static Vec3 GetForward(const Mat4& mat) { return{ mat(0, 2), mat(1, 2), mat(2, 2) }; }
	inline static Vec3 GetRight(const Mat4& mat) { return{ mat(0, 0), mat(1, 0), mat(2, 0) }; }

	inline static void MoveForward(Mat4& mat, float distance) {
		Vec3 offset = GetForward(mat);
		mat(0, 3) += offset[0] * distance;
		mat(1, 3) += offset[1] * distance;
		mat(2, 3) += offset[2] * distance;
	}
	inline static void MoveBack(Mat4& mat, float distance) { MoveForward(mat, -distance); }
	inline static void MoveRight(Mat4& mat, float distance) {
		Vec3 offset = GetRight(mat);
		mat(0, 3) += offset[0] * distance;
		mat(1, 3) += offset[1] * distance;
		mat(2, 3) += offset[2] * distance;
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

	inline static Mat4 InverseTranspose(Mat4 &M)
	{
		Mat4 A = M;
		//A.GetColumn(3) = { 0, 0, 0, 1 };

		return A.Inverse().Transpose();
	}
};

}
#endif // !__MATH_HELPER__
