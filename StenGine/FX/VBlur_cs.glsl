#version 450

const float gWeights[11] =
{
	0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
};

const int gBlurRadius = 5;

layout (binding=0, rgba16f) uniform image2D gInput;
layout (binding=1, rgba16f) uniform image2D gDeferredShaded;

#define N 256
#define CacheSize (N + 2*gBlurRadius)
shared vec4 gCache[CacheSize];

layout(local_size_x = N, local_size_y = 1, local_size_z = 1) in;

void main()
{
	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//

	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
	if (gl_LocalInvocationID.x < gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		uint x = max(gl_GlobalInvocationID.x - gBlurRadius, 0);
		gCache[gl_LocalInvocationID.x] =  imageLoad(gInput, ivec2(x, gl_GlobalInvocationID.y));	
	}
	if (gl_LocalInvocationID.x >= N - gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		uint x = min(gl_GlobalInvocationID.x + gBlurRadius, imageSize(gInput).x - 1);
		gCache[gl_LocalInvocationID.x + 2 * gBlurRadius] = imageLoad(gInput, ivec2(x, gl_GlobalInvocationID.y));
	}

	// Clamp out of bound samples that occur at image borders.
	gCache[gl_LocalInvocationID.x + gBlurRadius] = imageLoad(gInput, ivec2(min(gl_GlobalInvocationID.xy, imageSize(gInput).xy - 1)));

	// Wait for all threads to finish.
	groupMemoryBarrier();
	barrier();
	//
	// Now blur each pixel.
	//

	vec4 blurColor = vec4(0, 0, 0, 0);

	for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		uint k = gl_LocalInvocationID.x + gBlurRadius + i;

		blurColor += gWeights[i + gBlurRadius] * gCache[k];
	}

	imageStore(gDeferredShaded, ivec2(gl_GlobalInvocationID.xy), blurColor);
}