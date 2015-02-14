#define TERRAIN
#include "StandardConstant.fx"

TerrainVertexOut main(TerrainVertexIn vIn) {
	TerrainVertexOut vOut;
	
	vOut.PosW = vIn.PosL; // need PosV actually?
	vOut.PosW.y = gHeightMap.SampleLevel(gLinearMipPointSampler, vIn.TexUV, 0).r;

	vOut.PosW = mul(float4(vOut.PosW, 1.0f), gWorld).xyz;

	vOut.TexUV = vIn.TexUV;
	vOut.BoundsY = vIn.BoundsY;

	return vOut;
}