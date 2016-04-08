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

out TcsOut {
	vec3 pPosW;
	vec3 pNormalV;
	vec3 pNormalW;
	vec3 pTangV;
	vec2 pTexUV;
	vec4 pShadowTransform;
} tcsOut;

layout(std140) uniform ubPerObj {
	mat4 gWorldViewProj;
	mat4 gWorld;
	mat4 gWorldView;
	mat4 gViewProj;
	mat4 gShadowTransform;
	Material gMat;
	vec4 DiffX_NormY_ShadZ;
	
	sampler2D gDiffuseMap;
	sampler2D gNormalMap;
	sampler2D gShadowMap;
	sampler2D gBumpMap;
	samplerCube gCubeMap;
};

void main()
{
	// Interpolate patch attributes to generated vertices.
	tcsOut.pPosW = gl_TessCoord.x*tesIn[0].PosW + gl_TessCoord.y*tesIn[1].PosW + gl_TessCoord.z*tesIn[2].PosW;
	tcsOut.pNormalV = gl_TessCoord.x*tesIn[0].NormalV + gl_TessCoord.y*tesIn[1].NormalV + gl_TessCoord.z*tesIn[2].NormalV;
	tcsOut.pNormalW = gl_TessCoord.x*tesIn[0].NormalW + gl_TessCoord.y*tesIn[1].NormalW + gl_TessCoord.z*tesIn[2].NormalW;
	tcsOut.pTangV = gl_TessCoord.x*tesIn[0].TangentV + gl_TessCoord.y*tesIn[1].TangentV + gl_TessCoord.z*tesIn[2].TangentV;
	tcsOut.pTexUV = gl_TessCoord.x*tesIn[0].TexUV + gl_TessCoord.y*tesIn[1].TexUV + gl_TessCoord.z*tesIn[2].TexUV;
	tcsOut.pShadowTransform = gl_TessCoord.x*tesIn[0].ShadowPosH + gl_TessCoord.y*tesIn[1].ShadowPosH + gl_TessCoord.z*tesIn[2].ShadowPosH;

	// Interpolating normal can unnormalize it, so normalize it.
	tcsOut.pNormalV = normalize(tcsOut.pNormalV);
	tcsOut.pNormalW = normalize(tcsOut.pNormalW);

	//
	// Displacement mapping.
	//

	// Choose the mipmap level based on distance to the eye; specifically, choose
	// the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
	//const float MipInterval = 20.0f;
	//float mipLevel = clamp((distance(tcsOut.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);

	// Sample height map (stored in alpha channel).
	//float h = gBumpMap.SampleLevel(gDiffuseMapSampler, tcsOut.TexUV, 0).x;
	float h = textureLod(gBumpMap, tcsOut.pTexUV, 0).x;

	// Offset vertex along normal.
	tcsOut.pPosW += (/*gHeightScale*/ 0.2 *(h - 1.0))*tcsOut.pNormalW;

	// Project to homogeneous clip space.
	//tcsOut.PosH = mul(float4(tcsOut.PosW, 1.0f), gViewProj);
	gl_Position = gViewProj * vec4(tcsOut.pPosW, 1.0f);
}