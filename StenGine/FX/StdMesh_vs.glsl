#version 410

// input layout
layout(location = 0) in vec3 PosL;
layout(location = 1) in vec3 NormalL;
layout(location = 2) in vec2 TexUV;
layout(location = 3) in vec3 TangentL;

// constant buffer
uniform mat4 gWorldViewProj;

out vec2 pTexUV;

void main() {
	gl_Position = gWorldViewProj * vec4(PosL, 1.0);
	pTexUV = TexUV;
}
