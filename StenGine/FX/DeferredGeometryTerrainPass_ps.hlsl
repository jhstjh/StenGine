#define TERRAIN
#include "StandardConstant.fx"

PixelOut main(TerrainDomainOut pIn) {
	PixelOut pOut;

	pIn.ShadowPosH.xyz /= pIn.ShadowPosH.w;
	float depth = pIn.ShadowPosH.z;
	float shadowLit = 0;

	shadowLit += gShadowMap.SampleCmpLevelZero(gShadowSampler,
		pIn.ShadowPosH.xy, depth).r;

	float2 leftTex = pIn.TexUV + float2(-gTexelCellSpaceU, 0.0f);
	float2 rightTex = pIn.TexUV + float2(gTexelCellSpaceU, 0.0f);
	float2 bottomTex = pIn.TexUV + float2(0.0f, gTexelCellSpaceV);
	float2 topTex = pIn.TexUV + float2(0.0f, -gTexelCellSpaceV);

	float leftY = gHeightMap.SampleLevel(gLinearMipPointSampler, leftTex, 0).r;
	float rightY = gHeightMap.SampleLevel(gLinearMipPointSampler, rightTex, 0).r;
	float bottomY = gHeightMap.SampleLevel(gLinearMipPointSampler, bottomTex, 0).r;
	float topY = gHeightMap.SampleLevel(gLinearMipPointSampler, topTex, 0).r;

	float3 tangent = normalize(float3(2.f * gWorldCellSpace, rightY - leftY, 0.f));
	float3 bitan = normalize(float3(0.f, bottomY - topY, -2.f * gWorldCellSpace));
	float3 normalW = cross(tangent, bitan);

	pOut.normalV = mul(float4(normalW, 0.f), gView);
	pOut.normalV.w = 1.0f;

	pOut.normalV = (pOut.normalV + 1) / 2.0f;

	float4 c0 = gLayerMapArray.Sample(gDiffuseMapSampler, float3(pIn.TiledTex, 0.f));
	float4 c1 = gLayerMapArray.Sample(gDiffuseMapSampler, float3(pIn.TiledTex, 1.f));
	float4 c2 = gLayerMapArray.Sample(gDiffuseMapSampler, float3(pIn.TiledTex, 2.f));
	float4 c3 = gLayerMapArray.Sample(gDiffuseMapSampler, float3(pIn.TiledTex, 3.f));
	float4 c4 = gLayerMapArray.Sample(gDiffuseMapSampler, float3(pIn.TiledTex, 4.f));

	float4 t = gBlendMap.Sample(gDiffuseMapSampler, pIn.TexUV);

	float4 texColor = c0;
	texColor = lerp(texColor, c1, t.r);
	texColor = lerp(texColor, c2, t.g);
	texColor = lerp(texColor, c3, t.b);
	texColor = lerp(texColor, c4, t.a);

	pOut.diffuseH = texColor;
	pOut.diffuseH.w = saturate(shadowLit);
	pOut.specularH = float4(0, 0, 0, 0);

	return pOut;
}