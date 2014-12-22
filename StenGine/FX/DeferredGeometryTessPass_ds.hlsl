#include "StandardConstant.fx"

// The domain shader is called for every vertex created by the tessellator.  
// It is like the vertex shader after tessellation.
[domain("tri")]
DomainOut main(PatchTess patchTess,
	float3 bary : SV_DomainLocation,
	const OutputPatch<HullOut, 3> tri)
{
	DomainOut dout;

	// Interpolate patch attributes to generated vertices.
	dout.PosW = bary.x*tri[0].PosW + bary.y*tri[1].PosW + bary.z*tri[2].PosW;
	dout.NormalV = bary.x*tri[0].NormalV + bary.y*tri[1].NormalV + bary.z*tri[2].NormalV;
	dout.NormalW = bary.x*tri[0].NormalW + bary.y*tri[1].NormalW + bary.z*tri[2].NormalW;
	dout.TangentV = bary.x*tri[0].TangentV + bary.y*tri[1].TangentV + bary.z*tri[2].TangentV;
	dout.TexUV = bary.x*tri[0].TexUV + bary.y*tri[1].TexUV + bary.z*tri[2].TexUV;
	dout.ShadowPosH = bary.x*tri[0].ShadowPosH + bary.y*tri[1].ShadowPosH + bary.z*tri[2].ShadowPosH;

	// Interpolating normal can unnormalize it, so normalize it.
	dout.NormalV = normalize(dout.NormalV);
	dout.NormalW = normalize(dout.NormalW);

	//
	// Displacement mapping.
	//

	// Choose the mipmap level based on distance to the eye; specifically, choose
	// the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
	//const float MipInterval = 20.0f;
	//float mipLevel = clamp((distance(dout.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);

	// Sample height map (stored in alpha channel).
	float h = gBumpMap.SampleLevel(gDiffuseMapSampler, dout.TexUV, 0).x;

	// Offset vertex along normal.
	dout.PosW += (/*gHeightScale*/ 0.2 *(h - 1.0))*dout.NormalW;

	// Project to homogeneous clip space.
	dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj);
	return dout;
}