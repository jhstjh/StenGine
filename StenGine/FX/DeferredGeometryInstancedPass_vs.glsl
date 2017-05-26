#version 450
#extension GL_ARB_bindless_texture : require

// input layout
layout(location = 0) in vec3 PosL;
layout(location = 1) in vec3 NormalL;
layout(location = 2) in vec3 TangentL;
layout(location = 3) in vec2 TexUV;
layout(location = 4) in vec3 InstancePosition;

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

layout(std140) uniform ubPerObj{
	mat4 gWorldViewProj;
	mat4 pad0;
	mat4 pad1;
	mat4 gWorldView;
	mat4 gWorld;
	mat4 gViewProj;
	mat4 gShadowTransform;
	Material gMat;
	vec4 DiffX_NormY_ShadZ;
};

layout(std140) uniform ubPerFrame{
	vec4 gEyePosW;
	DirectionalLight gDirLight;
};

layout(std140) uniform ubTextures{
	sampler2D gDiffuseMap;
	sampler2D gNormalMap;
	sampler2D gShadowMap;
	sampler2D gBumpMap;
	samplerCube gCubeMap;
};

out VertexOut {
	vec2 pTexUV;
	vec3 pNormalV;
	vec3 pNormalW;
	vec3 pPosW;
	vec3 pTangV;
	vec4 pShadowTransform;
} vOut;

void main() {
    vec3 newPosL = PosL + InstancePosition;

	gl_Position = gWorldViewProj * vec4(newPosL, 1.0);
	vOut.pNormalV = vec3(gWorldView * vec4(NormalL, 0.0));
	vOut.pNormalW = vec3(gWorld * vec4(NormalL, 0.0));
	vOut.pTangV = vec3(gWorldView * vec4(TangentL, 0.0));
	vOut.pPosW = vec3(gWorld * vec4(newPosL, 1.0));
	vOut.pTexUV = TexUV;
	vOut.pShadowTransform = gShadowTransform * vec4(newPosL, 1.0);
}
