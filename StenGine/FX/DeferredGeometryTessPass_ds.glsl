// tess eval shader

#version 450
#extension GL_ARB_bindless_texture : require

layout(triangles, fractional_odd_spacing, ccw) in;

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

in TcsOut {
	vec3 PosW;
	vec3 NormalV;
	vec3 NormalW;
	vec3 TangentV;
	vec2 TexUV;
	vec4 ShadowPosH;
} tesIn[];

out TesOut {
	vec3 pPosW;
	vec3 pNormalV;
	vec3 pNormalW;
	vec3 pTangV;
	vec2 pTexUV;
	vec4 pShadowTransform;
} tesOut;

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

layout(std140) uniform ubTextures{
	sampler2D gDiffuseMap;
	sampler2D gNormalMap;
	sampler2D gShadowMap;
	sampler2D gBumpMap;
	samplerCube gCubeMap;
};

void main()
{
	// Interpolate patch attributes to generated vertices.
	tesOut.pPosW = gl_TessCoord.x*tesIn[0].PosW + gl_TessCoord.y*tesIn[1].PosW + gl_TessCoord.z*tesIn[2].PosW;
	tesOut.pNormalV = gl_TessCoord.x*tesIn[0].NormalV + gl_TessCoord.y*tesIn[1].NormalV + gl_TessCoord.z*tesIn[2].NormalV;
	tesOut.pNormalW = gl_TessCoord.x*tesIn[0].NormalW + gl_TessCoord.y*tesIn[1].NormalW + gl_TessCoord.z*tesIn[2].NormalW;
	tesOut.pTangV = gl_TessCoord.x*tesIn[0].TangentV + gl_TessCoord.y*tesIn[1].TangentV + gl_TessCoord.z*tesIn[2].TangentV;
	tesOut.pTexUV = gl_TessCoord.x*tesIn[0].TexUV + gl_TessCoord.y*tesIn[1].TexUV + gl_TessCoord.z*tesIn[2].TexUV;
	tesOut.pShadowTransform = gl_TessCoord.x*tesIn[0].ShadowPosH + gl_TessCoord.y*tesIn[1].ShadowPosH + gl_TessCoord.z*tesIn[2].ShadowPosH;

	// Interpolating normal can unnormalize it, so normalize it.
	tesOut.pNormalV = normalize(tesOut.pNormalV);
	tesOut.pNormalW = normalize(tesOut.pNormalW);

	//
	// Displacement mapping.
	//

	// Choose the mipmap level based on distance to the eye; specifically, choose
	// the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
	//const float MipInterval = 20.0f;
	//float mipLevel = clamp((distance(tesOut.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);

	// Sample height map (stored in alpha channel).
	//float h = gBumpMap.SampleLevel(gDiffuseMapSampler, tesOut.TexUV, 0).x;
	float h = textureLod(gBumpMap, tesOut.pTexUV, 0).x;

	// Offset vertex along normal.
	tesOut.pPosW += (/*gHeightScale*/ 0.2 *(h - 1.0))*tesOut.pNormalW;

	// Project to homogeneous clip space.
	//tesOut.PosH = mul(float4(tesOut.PosW, 1.0f), gViewProj);
	gl_Position = gViewProj * vec4(tesOut.pPosW, 1.0f);
}