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
};

struct VertexIn {
	float3 PosL  : POSITION;
    float3 NormalL : NORMAL;
};

struct VertexOut {
	float4 PosH  : SV_POSITION;
    float3 NormalW : NORMAL;
};

VertexOut VertShader(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.NormalW = vin.NormalL;
	// Just pass vertex color into the pixel shader.
    //vout.Color = vin.Color;
    
    return vout;
}

float4 PixShader(VertexOut pin) : SV_Target
{
	//return float4(-gDirLight.direction, 1);
	return clamp(dot(-gDirLight.direction, pin.NormalW), 0, 1) * gMaterial.diffuse * gDirLight.intensity;
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
