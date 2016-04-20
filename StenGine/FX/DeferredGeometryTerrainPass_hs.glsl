// tess control shader

#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_EXT_texture_array : enable

layout(vertices = 4) out;

in TerrainVertexOut{
	vec3 PosW;
	vec2 TexUV;
	vec2 BoundsY;
} tcsIn[];

out TcsOut {
	vec3 PosW;
	vec2 TexUV;
} tcsOut[];

layout(std140) uniform ubPerFrame{
	vec3 gEyePosW;

	float pad0;

	// When distance is minimum, the tessellation is maximum.
	// When distance is maximum, the tessellation is minimum.
	float gMinDist;
	float gMaxDist;

	// Exponents for power of 2 tessellation.  The tessellation
	// range is [2^(gMinTess), 2^(gMaxTess)].  Since the maximum
	// tessellation is 64, this means gMaxTess can be at most 6
	// since 2^6 = 64.
	float gMinTess;
	float gMaxTess;

	float gTexelCellSpaceU;
	float gTexelCellSpaceV;
	float gWorldCellSpace;

	float pad1;
	vec2 gTexScale/* = vec2(50.0f)*/;

	vec2 pad2;
	vec4 gWorldFrustumPlanes[6];
};

#define ID gl_InvocationID

float CalcTessFactor(in vec3 p)
{
	float d = distance(p, gEyePosW);

	// max norm in xz plane (useful to see detail levels from a bird's eye).
	//float d = max( abs(p.x-gEyePosW.x), abs(p.z-gEyePosW.z) );

	float s = clamp((d - gMinDist) / (gMaxDist - gMinDist), 0.f, 1.f);

	return pow(2, (mix(gMaxTess, gMinTess, s)));
}

void main()
{
	tcsOut[ID].PosW = tcsIn[ID].PosW;
	tcsOut[ID].TexUV = tcsIn[ID].TexUV;

	// frustum culling?..

	//if (false) {
	//
	//}
	//else {
		vec3 e0 = 0.5f*(tcsIn[0].PosW + tcsIn[2].PosW);
		vec3 e1 = 0.5f*(tcsIn[0].PosW + tcsIn[1].PosW);
		vec3 e2 = 0.5f*(tcsIn[1].PosW + tcsIn[3].PosW);
		vec3 e3 = 0.5f*(tcsIn[2].PosW + tcsIn[3].PosW);
		vec3 c = 0.25f*(tcsIn[0].PosW + tcsIn[1].PosW +
						  tcsIn[2].PosW + tcsIn[3].PosW);

		gl_TessLevelOuter[0] = CalcTessFactor(e0);
		gl_TessLevelOuter[1] = CalcTessFactor(e1);
		gl_TessLevelOuter[2] = CalcTessFactor(e2);
		gl_TessLevelOuter[3] = CalcTessFactor(e3);

		gl_TessLevelInner[0] = CalcTessFactor(c);
		gl_TessLevelInner[1] = gl_TessLevelInner[0];
	//}
}