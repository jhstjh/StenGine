// struct DirectionalLight {
// 	float4 intensity;
// 	float3 direction;
// 	float pad;
// };

#include "StandardConstant.fx"

PixelOut main(VertexOut pin)
{
	PixelOut pout;
	pin.PosW /= pin.PosW.w;

	float3 eyeRay = normalize(pin.PosW - gEyePosW);
	float3 refRay = reflect(eyeRay, pin.NormalW);
	//float3 refRay = refract(eyeRay, pin.NormalW, 1.5);

	pin.ShadowPosH.xyz /= pin.ShadowPosH.w;
	float depth = pin.ShadowPosH.z;
	float shadowLit = 1 - gDiffX_NormY_ShadZ.z;

	shadowLit += gShadowMap.SampleCmpLevelZero(gShadowSampler,
 		pin.ShadowPosH.xy, depth).r;



	pout.diffuseH =  ((1 - gDiffX_NormY_ShadZ.x) * gCubeMap.Sample(gDiffuseMapSampler, refRay) + gDiffX_NormY_ShadZ.x * gDiffuseMap.Sample(gDiffuseMapSampler, pin.TexUV)) * gMaterial.diffuse;

	pout.diffuseH.w = saturate(shadowLit);
	pout.specularH = gMaterial.specular;
	pout.specularH.w /= 255.0f;
	//pout.normalV = normalize(pin.NormalV).xy;
	pout.normalV.xyz = (normalize(pin.NormalV) + 1) / 2;

	if (gDiffX_NormY_ShadZ.y > 0) {
		float3 normalMapNormal = gNormalMap.Sample(gDiffuseMapSampler, pin.TexUV);
		normalMapNormal = 2.0f * normalMapNormal - 1.0;

		float3 N = normalize(pin.NormalV);
		float3 T = normalize(pin.TangentV - dot(pin.TangentV, N)*N);
		float3 B = cross(N, T);

		float3x3 TBN = float3x3(T, B, N);

		pout.normalV.xyz = (normalize(mul(normalMapNormal, TBN)) + 1) / 2;//normalize(pin.NormalV).xy;
	}
	//pout.edgeH = float4(1, 1, 1, 1); // implement edge detection later

	return pout;
}