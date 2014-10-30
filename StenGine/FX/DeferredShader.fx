// struct DirectionalLight {
// 	float4 intensity;
// 	float3 direction;
// 	float pad;
// };

struct Material {
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer cbPerObject {
	float4x4 gWorldViewProj; 
	float4x4 gWorldViewInvTranspose;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldView;
	float4x4 gWorld;
	float4x4 gViewProj;
	Material gMaterial;
	float4x4 gShadowTransform;
	int4 gDiffX_NormY_ShadZ;
};

cbuffer cbPerFrame {
	//DirectionalLight gDirLight;
	float3 gEyePosW;
};

Texture2D gDiffuseMap;
Texture2D gNormalMap;
Texture2D gShadowMap;
Texture2D gBumpMap;
TextureCube gCubeMap;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerComparisonState samShadow
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	ComparisonFunc = LESS;
};

struct VertexIn {
	float3 PosL  : POSITION;
    float3 NormalL : NORMAL0;
	float3 TangentL: TANGENT;
	float2 TexUV : TEXCOORD;
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

struct PixelOut {
	unorm half4 diffuseH: SV_TARGET0;
	half2 normalV: SV_TARGET1;
	unorm half4 specularH: SV_TARGET2;
	//float4 edgeH: SV_TARGET3;
	//float4 positionV: SV_TARGET3;
};

VertexOut VertShader(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.NormalV = mul(vin.NormalL, gWorldViewInvTranspose);
	vout.TangentV = mul(vin.TangentL, gWorldViewInvTranspose);
	vout.TexUV = vin.TexUV;
	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.NormalW = mul(vin.NormalL, gWorldInvTranspose);
	//vout.PosV = mul(float4(vin.PosL, 1.0f), gWorldView);
    return vout;
}

PixelOut PixShader(VertexOut pin)
{
	PixelOut pout;
	pin.PosW /= pin.PosW.w;

	float3 eyeRay = normalize(pin.PosW - gEyePosW);
	float3 refRay = reflect(eyeRay, pin.NormalW);
	//float3 refRay = refract(eyeRay, pin.NormalW, 1.5);

	pin.ShadowPosH.xyz /= pin.ShadowPosH.w;
	float depth = pin.ShadowPosH.z;
	float shadowLit = 1 - gDiffX_NormY_ShadZ.z;

	shadowLit += gShadowMap.SampleCmpLevelZero(samShadow,
		pin.ShadowPosH.xy, depth).r;



	pout.diffuseH = ((1 - gDiffX_NormY_ShadZ.x) * gCubeMap.Sample(samAnisotropic, refRay) + gDiffX_NormY_ShadZ.x * gDiffuseMap.Sample(samAnisotropic, pin.TexUV)) * gMaterial.diffuse;
	//pout.diffuseH = gCubeMap.Sample(samAnisotropic, refRay);
	pout.diffuseH.w = saturate(shadowLit);
	pout.specularH = gMaterial.specular;
	pout.specularH.w /= 255.0f;
	pout.normalV = normalize(pin.NormalV).xy;

	if (gDiffX_NormY_ShadZ.y > 0) {
		float3 normalMapNormal = gNormalMap.Sample(samAnisotropic, pin.TexUV);
			normalMapNormal = 2.0f * normalMapNormal - 1.0;

		float3 N = normalize(pin.NormalV);
			float3 T = normalize(pin.TangentV - dot(pin.TangentV, N)*N);
			float3 B = cross(N, T);

			float3x3 TBN = float3x3(T, B, N);

			pout.normalV = normalize(mul(normalMapNormal, TBN));//normalize(pin.NormalV).xy;
	}
	//pout.edgeH = float4(1, 1, 1, 1); // implement edge detection later

	return pout;
}

TessVertexOut TessVertexShader(VertexIn vin) {
	TessVertexOut vout;

	//vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.NormalV = mul(vin.NormalL, gWorldViewInvTranspose);
	vout.TangentV = mul(vin.TangentL, gWorldViewInvTranspose);
	vout.TexUV = vin.TexUV;
	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.NormalW = mul(vin.NormalL, gWorldInvTranspose);
	vout.TessFactor = 32;
	//vout.PosV = mul(float4(vin.PosL, 1.0f), gWorldView);
	return vout;
}

struct PatchTess
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess : SV_InsideTessFactor;
};

PatchTess PatchHS(InputPatch<TessVertexOut, 3> patch,
	uint patchID : SV_PrimitiveID)
{
	PatchTess pt;

	// Average tess factors along edges, and pick an edge tess factor for 
	// the interior tessellation.  It is important to do the tess factor
	// calculation based on the edge properties so that edges shared by 
	// more than one triangle will have the same tessellation factor.  
	// Otherwise, gaps can appear.
	pt.EdgeTess[0] = 0.5f*(patch[1].TessFactor + patch[2].TessFactor);
	pt.EdgeTess[1] = 0.5f*(patch[2].TessFactor + patch[0].TessFactor);
	pt.EdgeTess[2] = 0.5f*(patch[0].TessFactor + patch[1].TessFactor);
	pt.InsideTess = pt.EdgeTess[0];

	return pt;
}

struct HullOut
{
	float3 PosW		: POSITION;
	float3 NormalV  : NORMAL0;
	float3 NormalW	: NORMAL1;
	float3 TangentV : TANGENT;
	float2 TexUV : TEXCOORD0;
	float4 ShadowPosH: TEXCOORD1;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<TessVertexOut, 3> p,
	uint i : SV_OutputControlPointID,
	uint patchId : SV_PrimitiveID)
{
	HullOut hout;

	// Pass through shader.
	hout.PosW = p[i].PosW;
	hout.NormalV = p[i].NormalV;
	hout.NormalW = p[i].NormalW;
	hout.TangentV = p[i].TangentV;
	hout.TexUV = p[i].TexUV;
	hout.ShadowPosH = p[i].ShadowPosH;
	return hout;
}

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
DomainOut DS(PatchTess patchTess,
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
	float h = gBumpMap.SampleLevel(samLinear, dout.TexUV, 0).x;

	// Offset vertex along normal.
	dout.PosW += (/*gHeightScale*/ 0.2 *(h - 1.0))*dout.NormalW;

	// Project to homogeneous clip space.
	dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj);
	return dout;
}

PixelOut TessPixShader(DomainOut pin)
{
	PixelOut pout;
	//pin.PosW /= pin.PosW.w;

	float3 eyeRay = normalize(pin.PosW - gEyePosW);
	float3 refRay = reflect(eyeRay, pin.NormalW);
	//float3 refRay = refract(eyeRay, pin.NormalW, 1.5);

	pin.ShadowPosH.xyz /= pin.ShadowPosH.w;
	float depth = pin.ShadowPosH.z;
	float shadowLit = 1 - gDiffX_NormY_ShadZ.z;

	shadowLit += gShadowMap.SampleCmpLevelZero(samShadow,
	pin.ShadowPosH.xy, depth).r;
	
	pout.diffuseH = ((1 - gDiffX_NormY_ShadZ.x) * gCubeMap.Sample(samAnisotropic, refRay) + gDiffX_NormY_ShadZ.x * gDiffuseMap.Sample(samAnisotropic, pin.TexUV)) * gMaterial.diffuse;
	//pout.diffuseH = gCubeMap.Sample(samAnisotropic, refRay);
	pout.diffuseH.w = saturate(shadowLit);
	pout.specularH = gMaterial.specular;
	pout.specularH.w /= 255.0f;
	pout.normalV = normalize(pin.NormalV).xy;

	
	if (gDiffX_NormY_ShadZ.y > 0) {
		float3 normalMapNormal = gNormalMap.Sample(samAnisotropic, pin.TexUV);
			normalMapNormal = 2.0f * normalMapNormal - 1.0;

		float3 N = normalize(pin.NormalV);
			float3 T = normalize(pin.TangentV - dot(pin.TangentV, N)*N);
			float3 B = cross(N, T);

			float3x3 TBN = float3x3(T, B, N);

			pout.normalV = normalize(mul(normalMapNormal, TBN));//normalize(pin.NormalV).xy;
	}
	
	//pout.edgeH = float4(1, 1, 1, 1); // implement edge detection later

	return pout;
}

RasterizerState WireFrame {
	FillMode = Wireframe;
};

technique11 DeferredShaderTech
{
    pass P0
    {
		SetVertexShader(CompileShader(vs_5_0, VertShader()));
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_5_0, PixShader()));
		SetBlendState(0, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique11 DeferredShaderTessTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, TessVertexShader()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, TessPixShader()));
		SetBlendState(0, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		//SetRasterizerState(WireFrame);
	}
}