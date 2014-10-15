struct DirectionalLight {
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 direction;
	float pad;
};


cbuffer cbPerObject {
	float4x4 gWorldViewProj; 
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
	return /*gDirLight.ambient*/ + clamp(dot(-gDirLight.direction, pin.NormalW), 0, 1) * float4(0.8, 0.8, 0.8, 0);
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
