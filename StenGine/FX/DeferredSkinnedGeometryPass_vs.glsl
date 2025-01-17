#version 450
#extension GL_ARB_bindless_texture : require

// input layout
layout(location = 0) in vec3 PosL;
layout(location = 1) in vec3 NormalL;
layout(location = 2) in vec3 TangentL;
layout(location = 3) in vec2 TexUV;
layout(location = 4) in vec4 JointWeights;
layout(location = 5) in uvec4 JointIndices;

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

layout(std140) uniform ubPerObj
{
	mat4 gWorldViewProj;
	mat4 gPrevWorldViewProj;
	mat4 pad0;
	mat4 pad1;
	mat4 gWorldView;
	mat4 gWorld;
	mat4 gViewProj;
	mat4 gShadowTransform;
	Material gMat;
	vec4 DiffX_NormY_ShadZ;
};

layout(std140) uniform ubPerFrame
{
	vec4 gEyePosW;
};

layout(std140) uniform ubTextures
{
	sampler2D gDiffuseMap;
	sampler2D gNormalMap;
	sampler2D gShadowMap;
	sampler2D gBumpMap;
	samplerCube gCubeMap;
};

layout(std430, binding = 15) buffer MatrixPalette
{
	mat4 gMatrixPalette[];
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

	vec4 PosL0 =     gMatrixPalette[JointIndices.x] * vec4(PosL, 1.0);
	vec4 PosL1 =     gMatrixPalette[JointIndices.y] * vec4(PosL, 1.0);
	vec4 PosL2 =     gMatrixPalette[JointIndices.z] * vec4(PosL, 1.0);
	vec4 PosL3 =     gMatrixPalette[JointIndices.w] * vec4(PosL, 1.0);
	vec4 NormalL0 =  gMatrixPalette[JointIndices.x] * vec4(NormalL, 0.0);
	vec4 NormalL1 =  gMatrixPalette[JointIndices.y] * vec4(NormalL, 0.0);
	vec4 NormalL2 =  gMatrixPalette[JointIndices.z] * vec4(NormalL, 0.0);
	vec4 NormalL3 =  gMatrixPalette[JointIndices.w] * vec4(NormalL, 0.0);
	vec4 TangentL0 = gMatrixPalette[JointIndices.x] * vec4(TangentL, 0.0);
	vec4 TangentL1 = gMatrixPalette[JointIndices.y] * vec4(TangentL, 0.0);
	vec4 TangentL2 = gMatrixPalette[JointIndices.z] * vec4(TangentL, 0.0);
	vec4 TangentL3 = gMatrixPalette[JointIndices.w] * vec4(TangentL, 0.0);


	vec4 PosLBlend = PosL0 * JointWeights.x + PosL1 * JointWeights.y + PosL2 * JointWeights.z + PosL3 * JointWeights.w;

	gl_Position = gWorldViewProj * vec4(PosLBlend.xyz, 1.0);

	//gl_Position = gWorldViewProj * vec4(PosL, 1.0);



	vec4 NormalLBlend = NormalL0 * JointWeights.x + NormalL1 * JointWeights.y + NormalL2 * JointWeights.z + NormalL3 * JointWeights.w;



	vec4 TangentLBlend = TangentL0 * JointWeights.x + TangentL1 * JointWeights.y + TangentL2 * JointWeights.z + TangentL3 * JointWeights.w;

	vOut.pNormalV = vec3(gWorldView * vec4(NormalLBlend.xyz, 0.0));
	vOut.pNormalW = vec3(gWorld * vec4(NormalLBlend.xyz, 0.0));
	vOut.pTangV = vec3(gWorldView * vec4(TangentLBlend.xyz, 0.0));
	vOut.pPosW = vec3(gWorld * vec4(PosLBlend.xyz, 1.0));
	vOut.pTexUV = TexUV;
	vOut.pShadowTransform = gShadowTransform * vec4(PosLBlend.xyz, 1.0);
}
