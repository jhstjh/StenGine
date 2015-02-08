// struct DirectionalLight {
// 	float4 intensity;
// 	float3 direction;
// 	float pad;
// };

#include "StandardConstant.fx"

VertexOut main(SkinnedVertexIn vin)
{
	VertexOut vout;

	// THIS IS WRONG!!!

	float3 SkinnedPosL = (mul(float4(vin.PosL, 1.0f), gJointMatrixPalette[vin.JointIndices.x]) * vin.JointWeights.x +
						  mul(float4(vin.PosL, 1.0f), gJointMatrixPalette[vin.JointIndices.y]) * vin.JointWeights.y +
						  mul(float4(vin.PosL, 1.0f), gJointMatrixPalette[vin.JointIndices.z]) * vin.JointWeights.z +
						  mul(float4(vin.PosL, 1.0f), gJointMatrixPalette[vin.JointIndices.w]) * vin.JointWeights.w).xyz;
	float3 SkinnedNormalL = (mul(float4(vin.NormalL, 0.0f), gJointMatrixPalette[vin.JointIndices.x]) * vin.JointWeights.x +
							 mul(float4(vin.NormalL, 0.0f), gJointMatrixPalette[vin.JointIndices.y]) * vin.JointWeights.y +
							 mul(float4(vin.NormalL, 0.0f), gJointMatrixPalette[vin.JointIndices.z]) * vin.JointWeights.z +
							 mul(float4(vin.NormalL, 0.0f), gJointMatrixPalette[vin.JointIndices.w]) * vin.JointWeights.w).xyz;
	float3 SkinnedTangentL = (mul(float4(vin.TangentL, 0.0f), gJointMatrixPalette[vin.JointIndices.x]) * vin.JointWeights.x +
							  mul(float4(vin.TangentL, 0.0f), gJointMatrixPalette[vin.JointIndices.y]) * vin.JointWeights.y +
							  mul(float4(vin.TangentL, 0.0f), gJointMatrixPalette[vin.JointIndices.z]) * vin.JointWeights.z +
							  mul(float4(vin.TangentL, 0.0f), gJointMatrixPalette[vin.JointIndices.w]) * vin.JointWeights.w).xyz;

	vout.PosH = mul(float4(SkinnedPosL, 1.0f), gWorldViewProj);
	vout.NormalV = mul(SkinnedNormalL, gWorldViewInvTranspose);
	vout.TangentV = mul(SkinnedTangentL, gWorldViewInvTranspose);
	vout.TexUV = vin.TexUV;
	vout.ShadowPosH = mul(float4(SkinnedPosL, 1.0f), gShadowTransform);
	vout.PosW = mul(float4(SkinnedPosL, 1.0f), gWorld);
	vout.NormalW = mul(SkinnedNormalL, gWorldInvTranspose);
	//vout.PosV = mul(float4(vin.PosL, 1.0f), gWorldView);
    return vout;
}