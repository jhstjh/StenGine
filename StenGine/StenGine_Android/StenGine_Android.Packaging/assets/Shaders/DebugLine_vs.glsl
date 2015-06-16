#version 300 es

layout(location = 0) in vec3 PosW;
layout(location = 1) in vec4 ColorW;

out vec4 pColor;

layout(std140) uniform ubPerObj {
	mat4 gViewProj;
};

void main() {

	//vOut.Color = vIn.Color;
	gl_Position = gViewProj * vec4(PosW, 1.0f);
	pColor = ColorW;
}