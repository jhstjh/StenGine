#define TERRAIN
#include "StandardConstant.fx"

[domain("quad")]
TerrainDomainOut main(TerrainPatchTess patchTess,
					  float2 uv: SV_DomainLocation,
					  const OutputPatch<TerrainHullOut, 4> quad) 
{
	TerrainDomainOut dOut;

	dOut.PosW = lerp(
		lerp(quad[0].PosW, quad[1].PosW, uv.x),
		lerp(quad[2].PosW, quad[3].PosW, uv.x),
		uv.y);

	dOut.TexUV = lerp(
		lerp(quad[0].TexUV, quad[1].TexUV, uv.x),
		lerp(quad[2].TexUV, quad[3].TexUV, uv.x),
		uv.y);

	dOut.TiledTex = dOut.TexUV * gTexScale;

	dOut.PosW.y = gHeightMap.SampleLevel(gLinearMipPointSampler, dOut.TexUV, 0).r;

	dOut.PosH = mul(float4(dOut.PosW, 1.0f), gViewProj);
	dOut.ShadowPosH = mul(float4(dOut.PosW, 1.0f), gShadowTransform);

	return dOut;
}