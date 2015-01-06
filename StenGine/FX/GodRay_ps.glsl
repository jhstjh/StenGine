#version 410
#define NUM_SAMPLES 200

layout(std140) uniform ubPerFrame
{
	vec4 gLightPosH;
};

uniform sampler2D gOcclusionMap;

in vec2 pTexUV;
out vec4 svColor;

void main() {
	//return gOcclusionMap.Sample(gDiffuseMapSampler, pIn.Tex).w;
	if (1 - gLightPosH.z < 0)
		discard;

	float Density = 0.5;
	float Decay = 0.9;
	float Exposure = 0.2;
	float Weight = 0.5;

	vec2 texCoord = pTexUV;
		
	// Calculate vector from pixel to light source in screen space.  
	vec2 deltaTexCoord = (texCoord - gLightPosH.xy);

	Weight = (4.0 - clamp(abs(gLightPosH.x - 0.5), 0.0, 4.0)) / 4;
	//Weight = clamp(length(deltaTexCoord) / 10, 0, 1);

	deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;
	deltaTexCoord = clamp(deltaTexCoord, -0.02, 0.02);
	// Store initial sample.  
	float color = 0;
	float count = 0;

	float illuminationDecay = 1.0f;
	// Evaluate summation from Equation 3 NUM_SAMPLES iterations.  
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		// Step sample location along ray.  
		texCoord -= deltaTexCoord;
		// Retrieve sample at new location.  
		// sample is the inverse of occlusion map (1: occluded, 0: non-occluded)
		float samplePix = 1 - texture(gOcclusionMap, texCoord).w;

		//[flatten]
		if (length((texCoord - gLightPosH.xy) / vec2(0.5f, 0.85f)) > 0.1)
			samplePix /= 8;
		// Apply sample attenuation scale/decay factors.  
		samplePix *= illuminationDecay * Weight;
		// Accumulate combined color.  
		color += samplePix;
		// Update exponential decay factor.  
		illuminationDecay *= Decay;
	}
	// Output final color with a further scale control factor.  
	//color /= 5005;
	svColor =  vec4(color * Exposure, color * Exposure, color * Exposure, 1);
}