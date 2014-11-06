struct VSOut
{
	float4 pos : SV_Position;
	float2 Tex : TEXCOORD;
};

struct PSOut
{
	float4 DeferredShade: SV_TARGET0;
	float4 SSAO: SV_TARGET1;
};

cbuffer cbSettings: register(b0) {
	float2 texOffset;
	float2 pad;
}

cbuffer cbFixed: register(b13)
{
	static const float gWeights[21] =
	{
		0.013757049563821119, 0.01888216932766378, 0.025066978648142262, 0.03218663770378345, 0.03997355278034706, 0.048016821060535536, 0.05578758693508665, 0.06269100992275224, 0.06813911255361485, 0.07163267956032793, 0.07283656203947195, 0.07163267956032793, 0.06813911255361485, 0.06269100992275224, 0.05578758693508665, 0.048016821060535536, 0.03997355278034706, 0.03218663770378345, 0.025066978648142262, 0.01888216932766378, 0.013757049563821119,
	};

	static const int gBlurRadius = 10;
};

Texture2D gScreenMap: register(t0);
Texture2D gSSAOMap: register(t1);

SamplerState gSamplerStateLinear;

float4 main(VSOut input) : SV_Target
{
	return gScreenMap.Sample(gSamplerStateLinear, input.Tex) *  gSSAOMap.Sample(gSamplerStateLinear, input.Tex);
	// The center value always contributes to the sum.
	float4 color = gWeights[10] * gSSAOMap.Sample(gSamplerStateLinear, input.Tex);
	float totalWeight = gWeights[10];

// 	float centerDepth = gDepthGB.Sample(samAnisotropic, input.Tex);
// 
// 	float x = input.Tex.x * 2 - 1;
// 	float y = (1 - input.Tex.y) * 2 - 1;
// 	float4 vProjectedPos = float4(x, y, centerDepth, 1.0f);
// 	float4 vPositionVS = mul(vProjectedPos, gProjInv);
// 	vPositionVS.xyz /= vPositionVS.w;
// 
// 	float4 centerNormal = gNormalGB.Sample(samAnisotropic, input.Tex);

	for (float i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		// We already added in the center weight.
		if (i == 0)
			continue;

		float2 tex = input.Tex + i*texOffset;

// 		float neighborDepth = gDepthGB.Sample(samAnisotropic, tex);
// 		float4 neighborNormal = gNormalGB.Sample(samAnisotropic, tex);
// 
// 		float x = input.Tex.x * 2 - 1;
// 		float y = (1 - input.Tex.y) * 2 - 1;
// 		float4 nProjectedPos = float4(x, y, neighborDepth, 1.0f);
// 		float4 nPositionVS = mul(vProjectedPos, gProjInv);
// 		nPositionVS.xyz /= nPositionVS.w;

// 		if (dot(neighborNormal.xyz, centerNormal.xyz) >= 0.8f &&
//			abs(nPositionVS.z - vPositionVS.z) <= 0.2f)
		{
			float weight = gWeights[i + gBlurRadius];

			// Add neighbor pixel to blur.
			color += weight * gSSAOMap.Sample(gSamplerStateLinear, tex);

			totalWeight += weight;
		}
	}
	
	// Compensate for discarded samples by making total weights sum to 1.
	color /= totalWeight;
	if (texOffset.x > 0) return saturate(color) * gScreenMap.Sample(gSamplerStateLinear, input.Tex);
	else return color;
}

// 
// technique11 HBlurTech
// {
// 	pass p0
// 	{
// 		SetVertexShader(CompileShader(vs_5_0, VSmain()));
// 		SetGeometryShader(NULL);
// 		SetPixelShader(CompileShader(ps_5_0, PSBlurmain(float2(1.0/1280, 0.0f))));
// 		SetBlendState(0, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
// 	}
// }
// 
// technique11 VBlurTech
// {
// 	pass p0
// 	{
// 		SetVertexShader(CompileShader(vs_5_0, VSmain()));
// 		SetGeometryShader(NULL);
// 		SetPixelShader(CompileShader(ps_5_0, PSBlurmain(float2(0.0f, 1.0/720))));
// 		SetBlendState(0, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
// 	}
// }