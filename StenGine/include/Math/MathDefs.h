#pragma once

#include "mathfu/glsl_mappings.h"

namespace StenGine
{

using Vec2 = mathfu::vec2;
using Vec3 = mathfu::vec3;
using Vec4 = mathfu::vec4;
using Vec4ui = mathfu::Vector<uint32_t, 4>;

using Vec2Packed = mathfu::vec2_packed;
using Vec3Packed = mathfu::vec3_packed;
using Vec4Packed = mathfu::vec4_packed;
using Vec4uiPacked = mathfu::VectorPacked<uint32_t, 4>;

using Mat3 = mathfu::mat3;
using Mat4 = mathfu::mat4;

using Quat = mathfu::quat;

}