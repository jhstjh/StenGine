Texture2D gBumpMap: register(t2);
SamplerState gSamplerStateLinear: register(s0);

struct Material {
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer cbPerObject : register(b0) {
	float4x4 gWorldViewProj;
	float4x4 gWorldViewInvTranspose;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldView;
	float4x4 gWorld;
	float4x4 gViewProj;
	float4x4 gShadowTransform;
	Material gMaterial;
	int4 gDiffX_NormY_ShadZ;
};

struct PatchTess
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess : SV_InsideTessFactor;
};

struct HullOut
{
	float3 PosW		: POSITION;
	float3 NormalV  : NORMAL0;
	float3 NormalW	: NORMAL1;
	float3 TangentV : TANGENT;
	float2 TexUV : TEXCOORD0;
	float4 ShadowPosH: TEXCOORD1;
};

struct VertexOut {
	//float4 PosH  : SV_POSITION;
	float4 PosW  : POSITION;
	float3 NormalV : NORMAL0;
	float3 NormalW : NORMAL1;
	float3 TangentV: TANGENT;
	float2 TexUV : TEXCOORD0;
	float4 ShadowPosH: TEXCOORD1;
	float4 TessFactor: TESS;
};

struct DomainOut
{
	float4 PosH     : SV_POSITION;
	float3 PosW		: POSITION;
	float3 NormalV  : NORMAL0;
	float3 NormalW	: NORMAL1;
	float3 TangentV : TANGENT;
	float2 TexUV : TEXCOORD0;
	float4 ShadowPosH: TEXCOORD1;
};

// The domain shader is called for every vertex created by the tessellator.  
// It is like the vertex shader after tessellation.
[domain("tri")]
DomainOut main(PatchTess patchTess,
	float3 bary : SV_DomainLocation,
	const OutputPatch<HullOut, 3> tri)
{
	DomainOut dout;

	// Interpolate patch attributes to generated vertices.
	dout.PosW = bary.x*tri[0].PosW + bary.y*tri[1].PosW + bary.z*tri[2].PosW;
	dout.NormalV = bary.x*tri[0].NormalV + bary.y*tri[1].NormalV + bary.z*tri[2].NormalV;
	dout.NormalW = bary.x*tri[0].NormalW + bary.y*tri[1].NormalW + bary.z*tri[2].NormalW;
	dout.TangentV = bary.x*tri[0].TangentV + bary.y*tri[1].TangentV + bary.z*tri[2].TangentV;
	dout.TexUV = bary.x*tri[0].TexUV + bary.y*tri[1].TexUV + bary.z*tri[2].TexUV;
	dout.ShadowPosH = bary.x*tri[0].ShadowPosH + bary.y*tri[1].ShadowPosH + bary.z*tri[2].ShadowPosH;

	// Interpolating normal can unnormalize it, so normalize it.
	dout.NormalV = normalize(dout.NormalV);
	dout.NormalW = normalize(dout.NormalW);

	//
	// Displacement mapping.
	//

	// Choose the mipmap level based on distance to the eye; specifically, choose
	// the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
	//const float MipInterval = 20.0f;
	//float mipLevel = clamp((distance(dout.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);

	// Sample height map (stored in alpha channel).
	float h = gBumpMap.SampleLevel(gSamplerStateLinear, dout.TexUV, 0).x;

	// Offset vertex along normal.
	dout.PosW += (/*gHeightScale*/ 0.2 *(h - 1.0))*dout.NormalW;

	// Project to homogeneous clip space.
	dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj);
	return dout;
}