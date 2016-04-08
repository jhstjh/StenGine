// tess control shader

#version 450
layout(vertices = 3) out;

in TessVertexOut{
	vec2 pTexUV;
	vec3 pNormalV;
	vec3 pNormalW;
	vec3 pPosW;
	vec3 pTangV;
	vec4 pShadowTransform;
	float pTessFactor;
} tcsIn[];

out TcsOut {
	vec3 PosW;
	vec3 NormalV;
	vec3 NormalW;
	vec3 TangentV;
	vec2 TexUV;
	vec4 ShadowPosH;
} tcsOut[];

#define ID gl_InvocationID

void main()
{
	tcsOut[ID].PosW = tcsIn[ID].pPosW;
	tcsOut[ID].NormalV = tcsIn[ID].pNormalV;
	tcsOut[ID].NormalW = tcsIn[ID].pNormalW;
	tcsOut[ID].TangentV = tcsIn[ID].pTangV;
	tcsOut[ID].TexUV = tcsIn[ID].pTexUV;
	tcsOut[ID].ShadowPosH = tcsIn[ID].pShadowTransform;

	if (ID == 0)
	{
		gl_TessLevelOuter[0] = 0.5f*(tcsIn[1].pTessFactor + tcsIn[2].pTessFactor);
		gl_TessLevelOuter[1] = 0.5f*(tcsIn[2].pTessFactor + tcsIn[0].pTessFactor);
		gl_TessLevelOuter[2] = 0.5f*(tcsIn[0].pTessFactor + tcsIn[1].pTessFactor);
		gl_TessLevelInner[0] = gl_TessLevelOuter[0];
	}
}