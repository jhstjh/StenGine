// tess eval shader

#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_EXT_texture_array : enable

layout(quads, fractional_even_spacing, ccw) in;

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
	vec2 TexUV;
} tesIn[];

out TesOut {
	vec4 PosH;
	vec3 PosW;
	vec2 TexUV;
	vec2 TiledTex;
	vec4 ShadowPosH;
} tesOut;

layout(std140) uniform ubPerObj {
	mat4 gWorldViewProj;
	mat4 gWorldViewInvTranspose;
	mat4 gWorldInvTranspose;
	mat4 gWorldView;
	mat4 gWorld;
	mat4 gViewProj;
	mat4 gShadowTransform;
	mat4 gView;

	Material gMat;
	vec4 DiffX_NormY_ShadZ;
};

layout(std140) uniform ubPerFrame{
	vec3 gEyePosW;

	float pad0;

	// When distance is minimum, the tessellation is maximum.
	// When distance is maximum, the tessellation is minimum.
	float gMinDist;
	float gMaxDist;

	// Exponents for power of 2 tessellation.  The tessellation
	// range is [2^(gMinTess), 2^(gMaxTess)].  Since the maximum
	// tessellation is 64, this means gMaxTess can be at most 6
	// since 2^6 = 64.
	float gMinTess;
	float gMaxTess;

	float gTexelCellSpaceU;
	float gTexelCellSpaceV;
	float gWorldCellSpace;

	float pad1;
	vec2 gTexScale/* = vec2(50.0f)*/;

	vec2 pad2;
	vec4 gWorldFrustumPlanes[6];
};

layout(std140) uniform ubTextures {	
	sampler2D		gShadowMap;
	samplerCube		gCubeMap;
	sampler2D		gHeightMap;
	sampler2DArray	gLayerMapArray;
	sampler2D		gBlendMap;
};

void main()
{
	tesOut.PosW = mix(
		mix(tesIn[0].PosW, tesIn[1].PosW, gl_TessCoord.x),
		mix(tesIn[2].PosW, tesIn[3].PosW, gl_TessCoord.x),
		gl_TessCoord.y);

	tesOut.TexUV = mix(
		mix(tesIn[0].TexUV, tesIn[1].TexUV, gl_TessCoord.x),
		mix(tesIn[2].TexUV, tesIn[3].TexUV, gl_TessCoord.x),
		gl_TessCoord.y);

	tesOut.TiledTex = tesOut.TexUV * gTexScale;

	tesOut.PosW.y = textureLod(gHeightMap, tesOut.TexUV, 0).r;
	tesOut.ShadowPosH = gShadowTransform * vec4(tesOut.PosW, 1.0f);

	gl_Position = gViewProj * vec4(tesOut.PosW, 1.0f);
}