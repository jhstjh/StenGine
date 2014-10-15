struct DirectionalLight {
	float4 intensity;
	float3 direction;
	float pad;
};

struct Material {
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer cbPerObject {
	float4x4 gWorldViewProj; 
	Material gMaterial;
};

cbuffer cbPerFrame {
	DirectionalLight gDirLight;
	float3 gEyePosW;
};

Texture2D gDiffuseMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn {
	float3 PosL  : POSITION;
    float3 NormalL : NORMAL;
	float3 TexUV : TEXCOORD;
};

struct VertexOut {
	float4 PosH  : SV_POSITION;
	float3 PosW  : POSITION;
    float3 NormalW : NORMAL;
	float3 TexUV : TEXCOORD;
};

VertexOut VertShader(VertexIn vin)
{
	VertexOut vout;
	
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.NormalW = vin.NormalL;
	vout.PosW = vin.PosL;
	vout.TexUV = vin.TexUV;

    return vout;
}

float4 PixShader(VertexOut pin) : SV_Target
{
	pin.NormalW = normalize(pin.NormalW);

	float4 diffSpecColor = float4(0, 0, 0, 0);
	float diffuseK = dot(-gDirLight.direction, pin.NormalW);

	[flatten]
	if (diffuseK > 0) {
		diffSpecColor += diffuseK * gMaterial.diffuse * gDirLight.intensity;
		float3 refLight = reflect(-gDirLight.direction, pin.NormalW);
		float3 viewRay = gEyePosW - pin.PosW;
		viewRay = normalize(viewRay);
		diffSpecColor += gMaterial.specular * pow(max(dot(refLight, viewRay), 0), gMaterial.specular.w);
	}

	return  (gMaterial.ambient + diffSpecColor) * gDiffuseMap.Sample(samAnisotropic, pin.TexUV);
}

technique11 StdMeshTech
{
    pass P0
    {
		SetVertexShader(CompileShader(vs_5_0, VertShader()));
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_5_0, PixShader()));
    }
}
