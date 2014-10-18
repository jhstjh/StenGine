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
	float4x4 gWorldInvTranspose;
	float4x4 gWorld;
	Material gMaterial;
	float4x4 gShadowTransform;
};

cbuffer cbPerFrame {
	//DirectionalLight gDirLight;
	float3 gEyePosW;
};

Texture2D gDiffuseMap;
Texture2D gShadowMap;

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
    float3 NormalL : NORMAL;
	float2 TexUV : TEXCOORD;
};

struct VertexOut {
	float4 PosH  : SV_POSITION;
	float3 PosW  : POSITION;
    float3 NormalW : NORMAL;
	float2 TexUV : TEXCOORD0;
	float4 ShadowPosH: TEXCOORD1;
};

struct PixelOut {
	float4 diffuseH: SV_TARGET0;
	float4 positionW: SV_TARGET1;
	float4 normalW: SV_TARGET2;
	float4 specularH: SV_TARGET3;
};

VertexOut VertShader(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.NormalW = mul(vin.NormalL, gWorldInvTranspose);
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.TexUV = vin.TexUV;
	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);

    return vout;
}

PixelOut PixShader(VertexOut pin)
{
	PixelOut pout;

	pin.ShadowPosH.xyz /= pin.ShadowPosH.w;
	float depth = pin.ShadowPosH.z;
	float shadowLit = 0;

	shadowLit += gShadowMap.SampleCmpLevelZero(samShadow,
		pin.ShadowPosH.xy, depth).r;

	pout.diffuseH = gDiffuseMap.Sample(samAnisotropic, pin.TexUV) * gMaterial.diffuse;
	pout.diffuseH.w = shadowLit;
	pout.positionW = float4(pin.PosW, 1);
	pout.specularH = gMaterial.specular;
	pout.normalW = float4(pin.NormalW, 0);

	return pout;
}

technique11 DeferredShaderTech
{
    pass P0
    {
		SetVertexShader(CompileShader(vs_5_0, VertShader()));
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_5_0, PixShader()));
    }
}
