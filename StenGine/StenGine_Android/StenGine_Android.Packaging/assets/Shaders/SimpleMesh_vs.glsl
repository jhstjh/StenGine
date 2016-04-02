#version 300 es

// input layout
layout(location = 0) in vec3 PosL;
layout(location = 1) in vec3 NormalL;
layout(location = 2) in vec2 TexUV;
layout(location = 3) in vec3 TangentL;

struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 roughness_metalic_c_doublesided;
};

struct DirectionalLight {
	vec4 intensity;
	vec3 direction;
	float pad;
};

layout(std140) uniform ubPerObj {
	mat4 gWorldViewProj;
	mat4 gWorld;
	mat4 gWorldView;
	mat4 gShadowTransform;
	Material gMat;
	vec4 DiffX_NormY_ShadZ;
};

layout(std140) uniform ubPerFrame{
	vec4 gEyePosW;
	DirectionalLight gDirLight;
};

//out VertexOut {
//	vec2 pTexUV;
//	//vec3 pNormalV;
out vec3 pNormalW;
out vec3 pPosW;
//	//vec3 pTangV;
//	//vec4 pShadowTransform;
//} vOut;

void main() {
	gl_Position = gWorldViewProj * vec4(PosL, 1.0);
	pNormalW = vec3(gWorld * vec4(NormalL, 0.0));  // normal is inverted in opengl es
	pPosW = vec3(gWorld * vec4(PosL, 1.0));

	//vOut.pNormalV = vec3(gWorldView * vec4(NormalL, 0.0));
	//vOut.pNormalW = vec3(gWorld * vec4(NormalL, 0.0));
	//vOut.pTangV = vec3(gWorldView * vec4(TangentL, 0.0));
	//vOut.pPosW = vec3(gWorld * vec4(NormalL, 1.0));
	//vOut.pTexUV = TexUV;
	//vOut.pShadowTransform = gShadowTransform * vec4(PosL, 1.0);
}