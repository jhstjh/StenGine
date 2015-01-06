#define NUM_SAMPLES 200
#define GOD_RAY
#include "StandardConstant.fx"


Texture2D gOcclusionMap : register(t0);

float4 main(ScreenQuadVSOut pIn) : SV_TARGET
{
	//return gOcclusionMap.Sample(gDiffuseMapSampler, pIn.Tex).w;
	clip(1 - gLightPosH.z);

	float Density = 0.5;
	float Decay = 0.9;
	float Exposure = 0.2;
	float Weight = 0.5;

	float2 texCoord = pIn.Tex;
		
	// Calculate vector from pixel to light source in screen space.  
	half2 deltaTexCoord = (texCoord - gLightPosH.xy);

	Weight = (4.0 - clamp(abs(gLightPosH.x - 0.5), 0, 4.0)) / 4;
	//Weight = clamp(length(deltaTexCoord) / 10, 0, 1);

	deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;
	deltaTexCoord = clamp(deltaTexCoord, -0.02, 0.02);
	// Store initial sample.  
	half color = 0;
	half count = 0;

	half illuminationDecay = 1.0f;
	// Evaluate summation from Equation 3 NUM_SAMPLES iterations.  
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		// Step sample location along ray.  
		texCoord -= deltaTexCoord;
		// Retrieve sample at new location.  
		// sample is the inverse of occlusion map (1: occluded, 0: non-occluded)
		half sample = 1 - gOcclusionMap.Sample(gDiffuseMapSampler, texCoord).w;

		[flatten]
		if (length((texCoord - gLightPosH.xy) / float2(0.5f, 0.85f)) > 0.1)
			sample /= 8;
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