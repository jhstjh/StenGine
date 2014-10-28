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
	float3 TangentL: TANGENT;
	float2 TexUV : TEXCOORD;
};

struct VertexOut {
	float4 PosH  : SV_POSITION;
	//float4 PosV  : POSITION;
	float3 NormalV : NORMAL;
	float3 TangentV: TANGENT;
	float2 TexUV : TEXCOORD0;
	float4 ShadowPosH: TEXCOORD1;
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
	//vout.PosV = mul(float4(vin.PosL, 1.0f), gWorldView);
    return vout;
}

PixelOut PixShader(VertexOut pin)
{
	PixelOut pout;

	pin.ShadowPosH.xyz /= pin.ShadowPosH.w;
	float depth = pin.ShadowPosH.z;
	float shadowLit = 1 - gDiffX_NormY_ShadZ.z;

	shadowLit += gShadowMap.SampleCmpLevelZero(samShadow,
		pin.ShadowPosH.xy, depth).r;

	pout.diffuseH = ((1 - gDiffX_NormY_ShadZ.x) * float4(1.0, 1.0, 1.0, 1) + gDiffX_NormY_ShadZ.x * gDiffuseMap.Sample(samAnisotropic, pin.TexUV)) * gMaterial.diffuse;
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
