#version 410

layout(location = 0) in vec3 PosL;

uniform mat4 gWorldViewProj;

void main() {
	gl_Position = gWorldViewProj * vec4(PosL, 1.0);
}
