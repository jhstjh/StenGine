struct DirectionalLight {
	float4 intensity;
	float3 direction;
	float pad;
};

cbuffer cbPerFrame {
	DirectionalLight gDirLight;
	float3 gEyePosW;
};

struct VSOut
{
	float4 pos : SV_Position;
	float2 Tex : TEXCOORD;
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

Texture2D gScreenMap;

Texture2D gDiffuseGB;
Texture2D gPositionGB;
Texture2D gNormalGB;
Texture2D gSpecularGB;

VSOut VSmain(uint vertexId : SV_VertexID)
{
	VSOut output;

	if (vertexId == 0) {
		output.pos = float4(-1.0, -1.0, 0.0, 1.0);
		output.Tex = float2(0, 1);
	}
	else if (vertexId == 1) {
		output.pos = float4(-1.0, 1.0, 0.0, 1.0);
		output.Tex = float2(0, 0);
	}
	else if (vertexId == 2) {
		output.pos = float4(1.0, 1.0, 0.0, 1.0);
		output.Tex = float2(1, 0);
	}
	else if (vertexId == 3) {
		output.pos = float4(1.0, 1.0, 0.0, 1.0);
		output.Tex = float2(1, 0);
	}
	else if (vertexId == 4) {
		output.pos = float4(1.0, -1.0, 0.0, 1.0);
		output.Tex = float2(1, 1);
	}
	else if (vertexId == 5) {
		output.pos = float4(-1.0, -1.0, 0.0, 1.0);
		output.Tex = float2(0, 1);
	}
	
	return output;
}



struct PSIn
{
	float4 pos : SV_Position;
	linear float2 Tex : TEXCOORD;
};


float4 PSmain(PSIn input) : SV_Target
{
	// 	PSOut output;
	// 
	// 	output.color = color;
	float4 diffColor = float4(0, 0, 0, 0);
	float4 specColor = float4(0, 0, 0, 0);

	float3 normalW = gNormalGB.Sample(samAnisotropic, input.Tex).xyz;
	float diffuseK = dot(-gDirLight.direction, normalW);
	float shadowLit = gDiffuseGB.Sample(samAnisotropic, input.Tex).w;

	if (diffuseK > 0) {
		diffColor += diffuseK * gDirLight.intensity;
		float3 refLight = reflect(gDirLight.direction, normalW);
		float3 viewRay = gEyePosW - gPositionGB.Sample(samAnisotropic, input.Tex).xyz;
		viewRay = normalize(viewRay);
		specColor += gSpecularGB.Sample(samAnisotropic, input.Tex) * pow(max(dot(refLight, viewRay), 0), gSpecularGB.Sample(samAnisotropic, input.Tex).w);
	}

	return  (float4(0.2, 0.2, 0.2, 0) + diffColor * shadowLit) * gDiffuseGB.Sample(samAnisotropic, input.Tex) + specColor * shadowLit;
	//return diffuseColor * (clamp(diffuseK * gDirLight.intensity, 0, 1) + float4(0.2, 0.2, 0.2, 0));
}


technique11 t0
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, VSmain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSmain()));
	}
}

technique11 DeferredLightingTech
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, VSmain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSmain()));
	}
}