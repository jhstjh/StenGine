#version 410

layout(location = 0) in vec3 PosL;
layout(location = 1) in vec3 NormalL;
layout(location = 2) in vec2 TexUV;
layout(location = 3) in vec3 TangentL;

uniform mat4 gWorldViewProj;

void main() {
	mat4 test = gWorldViewProj;
	gl_Position = gWorldViewProj * vec4(PosL, 1.0);
}
