#ifndef __MATH_HELPER__
#define __MATH_HELPER__

#include "Graphics/Abstraction/RendererBase.h"

namespace StenGine
{

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

class MatrixHelper {
public:

	inline static void SetPosition(Mat4 &mat, float x, float y, float z) { mat(0, 3) = x; mat(1, 3) = y; mat(2, 3) = z; }
	inline static Vec3 GetPosition(Mat4 &mat) { return { mat(0, 3), mat(1, 3), mat(2,3)}; }
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

	inline static Mat4 InverseTranspose(Mat4 &M)
	{
		Mat4 A = M;
		return A.Inverse().Transpose();
	}
};

}
#endif // !__MATH_HELPER__
