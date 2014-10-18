cbuffer cbPerFrame {
	float3 gLightPosH;
};

Texture2D gOcclusionMap;

struct VSOut
{
	float4 pos : SV_Position;
	float2 Tex : TEXCOORD;
};

SamplerState samAnisotropic
{
	Filter = MIN_MAG_POINT_MIP_LINEAR;
	//MaxAnisotropy = 4;
	BorderColor = float4(0, 0, 0, 0);
	AddressU = BORDER;
	AddressV = BORDER;
};

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

static const int NUM_SAMPLES = 400;

float4 PSmain(PSIn input) : SV_Target
{
	float Density = 1;
	float Decay = 0.9;
	float Exposure = 0.1;
	float Weight = 0.5;

//	if (gLightPosH.z < 0 || gLightPosH.z > 1) return float4(0, 0, 0, 0);

// 	if (gLightPosH.x < 0 ||
// 		gLightPosH.y < 0 ||
// 		gLightPosH.x > 1 ||
// 		gLightPosH.y > 1 )
// 		return float4(0, 0, 0, 0);

// 	float occlusion = gOcclusionMap.Sample(samAnisotropic, input.Tex).w;
// 
// 	if (occlusion == 1) {
// 		half2 deltaTexCoord = (input.Tex - gLightPosH.xy);
// 			if (length(deltaTexCoord) < 0.1)
// 				return float4(1, 0, 0, 0);
// 			else
// 				return float4(0, 1, 0, 0);
// 	}
// 	else
// 		return float4(0, 0, 0, 0);



	float2 texCoord = input.Tex;

	// Calculate vector from pixel to light source in screen space.  
	half2 deltaTexCoord = (texCoord - gLightPosH.xy);
	// Divide by number of samples and scale by control factor.  
	deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;
	// Store initial sample.  
	half color = 0;

	//return float4(color, color, color, 1);
	// Set up illumination decay factor.  
	half illuminationDecay = 1.0f;
	// Evaluate summation from Equation 3 NUM_SAMPLES iterations.  
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		// Step sample location along ray.  
		texCoord -= deltaTexCoord;
		// Retrieve sample at new location.  
		half sample = gOcclusionMap.Sample(samAnisotropic, texCoord).w;
		// Apply sample attenuation scale/decay factors.  
		sample *= illuminationDecay * Weight;
		// Accumulate combined color.  
		color += sample;
		// Update exponential decay factor.  
		illuminationDecay *= Decay;
	}
	// Output final color with a further scale control factor.  
	//color /= 5005;
	return float4(color * Exposure, color * Exposure, color * Exposure, 1);
}

BlendState SrcAlphaBlendingAdd
{
	BlendEnable[0] = TRUE;
	SrcBlend = ONE;
	DestBlend = ONE;
	BlendOp = ADD;
	RenderTargetWriteMask[0] = 0x0F;
};

technique11 t0
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, VSmain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSmain()));
		SetBlendState(SrcAlphaBlendingAdd, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
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