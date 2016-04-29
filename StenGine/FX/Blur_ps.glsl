#version 450
#extension GL_ARB_bindless_texture : require

in vec2 pTexUV;
out vec4 ps_color;

layout(std140) uniform cbSettings {
	vec2 texOffset;
	vec2 pad;
	sampler2D gScreenMap;
	sampler2D gSSAOMap;
	sampler2D gBloomMap;
	sampler2D gDepthMap;
};

const float gWeights[21] =
{
	0.013757049563821119, 0.01888216932766378, 0.025066978648142262, 0.03218663770378345, 0.03997355278034706, 0.048016821060535536, 0.05578758693508665, 0.06269100992275224, 0.06813911255361485, 0.07163267956032793, 0.07283656203947195, 0.07163267956032793, 0.06813911255361485, 0.06269100992275224, 0.05578758693508665, 0.048016821060535536, 0.03997355278034706, 0.03218663770378345, 0.025066978648142262, 0.01888216932766378, 0.013757049563821119,
};

const int gBlurRadius = 10;


void main()
{
	vec4 originalColor = texture(gScreenMap, pTexUV);

	//vec4 bloomColor = texture(gBloomMap, pTexUV);
	gl_FragDepth = texture(gDepthMap, pTexUV).x;

	ps_color = originalColor * texture(gSSAOMap, pTexUV);

#if 0
	return (1 - (1 -originalColor)*(1 - bloomColor)) * gSSAOMap.Sample(gSamplerStateLinear, input.Tex);

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
#endif
}