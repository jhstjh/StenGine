#ifndef __STANDARD_CONSTANT__
#define __STANDARD_CONSTANT__

Texture2D		gDiffuseMap		:	register(t0);
Texture2D		gNormalMap		:	register(t1);
Texture2D		gShadowMap		:	register(t3);
Texture2D		gBumpMap		:	register(t2);
TextureCube		gCubeMap		:	register(t4);
Texture2D		gHeightMap		:	register(t5);
Texture2DArray	gLayerMapArray	:	register(t6);
Texture2D		gBlendMap		:	register(t7);

SamplerState gDiffuseMapSampler: register(s0);
SamplerComparisonState gShadowSampler: register(s1);
SamplerState gLinearMipPointSampler: register(s2);

struct Material {
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 roughness_metalic_c_doublesided;
};

struct DirectionalLight {
	float4 intensity;
	float3 direction;
	float pad;
};

#ifdef PRIMITIVE_LINE
cbuffer cbPerObject : register(b0) {
	float4x4 gViewProj;
};
//#elif defined PRIMITIVE_TRIANGLE
#else
cbuffer cbPerObject : register(b0) {
	float4x4 gWorldViewProj;
	float4x4 gWorldViewInvTranspose;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldView;
	float4x4 gWorld;
	float4x4 gViewProj;
	float4x4 gShadowTransform;
	Material gMaterial;
	float4 gDiffX_NormY_ShadZ;
};
#endif

cbuffer cbPerFrame : register(b1) {
#ifdef GOD_RAY
	float4 gLightPosH;
#elif defined TERRAIN
	float3 gEyePosW;

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
	float2 gTexScale = 50.0f;

	float2 pad2;
	float4 gWorldFrustumPlanes[6];
#else
	float4 gEyePosW;
#endif
};

// TODO: should be in a tbuffer. tmp cbuffer for now
cbuffer cbMatrixPalette : register(b2) {
	float4x4 gJointMatrixPalette[96];
}

struct VertexIn {
	float3 PosL		: POSITION;
	float3 NormalL	: NORMAL0;
	float3 TangentL	: TANGENT;
	float2 TexUV	: TEXCOORD;
};

struct SkinnedVertexIn {
	float3 PosL		: POSITION;
	float3 NormalL	: NORMAL0;
	float3 TangentL	: TANGENT;
	float2 TexUV	: TEXCOORD;
	float4 JointWeights : JOINTWEIGHTS;
	uint4  JointIndices : JOINTINDICES;
};

struct DebugLineVertexIn {
	float3 PosW : POSITION;
	//float4 Color : COLOR;
};

struct TerrainVertexIn {
	float3 PosL		: POSITION;
	float2 TexUV	: TEXCOORD0;
	float2 BoundsY	: TEXCOORD1;
};

struct VertexOut {
	float4 PosH  : SV_POSITION;
	float4 PosW  : POSITION;
	float3 NormalV : NORMAL0;
	float3 NormalW : NORMAL1;
	float3 TangentV: TANGENT;
	float2 TexUV : TEXCOORD0;
	float4 ShadowPosH: TEXCOORD1;
};

struct ScreenQuadVSOut {
	float4 pos : SV_Position;
	float2 Tex : TEXCOORD;
};

struct DebugLineVertexOut {
	float4 PosH : SV_POSITION;
	float4 Color : COLOR;
};

struct TessVertexOut {
	//float4 PosH  : SV_POSITION;
	float4 PosW  : POSITION;
	float3 NormalV : NORMAL0;
	float3 NormalW : NORMAL1;
	float3 TangentV: TANGENT;
	float2 TexUV : TEXCOORD0;
	float4 ShadowPosH: TEXCOORD1;
	float4 TessFactor: TESS;
};

struct TerrainVertexOut {
	float3 PosW		: POSITION;
	float2 TexUV	: TEXCOORD0;
	float2 BoundsY	: TEXCOORD1;
};

struct PixelOut {
	half4 diffuseH: SV_TARGET0;
	half4 normalV: SV_TARGET1;
	half4 specularH: SV_TARGET2;
	//float4 edgeH: SV_TARGET3;
	//float4 positionV: SV_TARGET3;
};

struct HullOut {
	float3 PosW		: POSITION;
	float3 NormalV  : NORMAL0;
	float3 NormalW	: NORMAL1;
	float3 TangentV : TANGENT;
	float2 TexUV : TEXCOORD0;
	float4 ShadowPosH: TEXCOORD1;
};

struct TerrainHullOut {
	float3 PosW		: POSITION;
	float2 TexUV	: TEXCOORD0;
};

struct DomainOut {
	float4 PosH     : SV_POSITION;
	float3 PosW		: POSITION;
	float3 NormalV  : NORMAL0;
	float3 NormalW	: NORMAL1;
	float3 TangentV : TANGENT;
	float2 TexUV : TEXCOORD0;
	float4 ShadowPosH: TEXCOORD1;
};

struct TerrainDomainOut {
	float4 PosH		: SV_POSITION;
	float3 PosW		: POSITION;
	float2 TexUV	: TEXCOORD0;
	float2 TiledTex	: TEXCOORD1;
};

struct PatchTess {
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess : SV_InsideTessFactor;
};

struct TerrainPatchTess {
	float EdgeTess[4]	: SV_TessFactor;
	float InsideTess[2]	: SV_InsideTessFactor;
};

#endif