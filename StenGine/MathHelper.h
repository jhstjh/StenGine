#ifndef __MATH_HELPER__
#define __MATH_HELPER__

#include <xnamath.h>

namespace MatrixHelper {

	XMFLOAT3 GetPosition(const XMFLOAT4X4& mat) { return XMFLOAT3(mat._41, mat._42, mat._43); }
	XMFLOAT3 NormalizeFloat3(XMFLOAT3& vec3) {
		double len = sqrt(vec3.x * vec3.x + vec3.y * vec3.y + vec3.z * vec3.z);
		vec3.x /= len;
		vec3.y /= len;
		vec3.z /= len;
		return vec3;
	}
}


#endif // !__MATH_HELPER__
