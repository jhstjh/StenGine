#version 410

// input layout
layout(location = 0) in vec3 PosL;
layout(location = 1) in vec3 NormalL;
layout(location = 2) in vec2 TexUV;
layout(location = 3) in vec3 TangentL;

// constant buffer
uniform mat4 gWorldViewProj;
uniform mat4 gWorld;

out vec2 pTexUV;
out vec3 pNormalW;
out vec3 pPosW;

void main() {
	gl_Position = gWorldViewProj * vec4(PosL, 1.0);
	pNormalW = vec3(gWorld * vec4(NormalL, 0.0));
	pPosW = vec3(gWorld * vec4(NormalL, 1.0));
	pTexUV = TexUV;
}
