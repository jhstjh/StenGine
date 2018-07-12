// struct DirectionalLight {
// 	float4 intensity;
// 	float3 direction;
// 	float pad;
// };

#include "StandardConstant.fx"

VertexOut main(SkinnedVertexIn vin)
{
	VertexOut vout;

	float4 PosL0 =     mul(float4(vin.PosL, 1.0)	, gMatrixPalette[vin.JointIndices.x]); 
	float4 PosL1 =     mul(float4(vin.PosL, 1.0)	, gMatrixPalette[vin.JointIndices.y]); 
	float4 PosL2 =     mul(float4(vin.PosL, 1.0)	, gMatrixPalette[vin.JointIndices.z]); 
	float4 PosL3 =     mul(float4(vin.PosL, 1.0)	, gMatrixPalette[vin.JointIndices.w]); 
	float4 NormalL0 =  mul(float4(vin.NormalL, 0.0)	, gMatrixPalette[vin.JointIndices.x]); 
	float4 NormalL1 =  mul(float4(vin.NormalL, 0.0)	, gMatrixPalette[vin.JointIndices.y]); 
	float4 NormalL2 =  mul(float4(vin.NormalL, 0.0)	, gMatrixPalette[vin.JointIndices.z]); 
	float4 NormalL3 =  mul(float4(vin.NormalL, 0.0)	, gMatrixPalette[vin.JointIndices.w]); 
	float4 TangentL0 = mul(float4(vin.TangentL, 0.0), gMatrixPalette[vin.JointIndices.x]); 
	float4 TangentL1 = mul(float4(vin.TangentL, 0.0), gMatrixPalette[vin.JointIndices.y]); 
	float4 TangentL2 = mul(float4(vin.TangentL, 0.0), gMatrixPalette[vin.JointIndices.z]); 
	float4 TangentL3 = mul(float4(vin.TangentL, 0.0), gMatrixPalette[vin.JointIndices.w]); 


	float4 PosLBlend = PosL0 * vin.JointWeights.x + PosL1 * vin.JointWeights.y + PosL2 * vin.JointWeights.z + PosL3 * vin.JointWeights.w;
	float4 NormalLBlend = NormalL0 * vin.JointWeights.x + NormalL1 * vin.JointWeights.y + NormalL2 * vin.JointWeights.z + NormalL3 * vin.JointWeights.w;
	float4 TangentLBlend = TangentL0 * vin.JointWeights.x + TangentL1 * vin.JointWeights.y + TangentL2 * vin.JointWeights.z + TangentL3 * vin.JointWeights.w;


	vout.PosH = mul(float4(PosLBlend.xyz, 1.0f), gWorldViewProj);
	vout.PosH2 = vout.PosH;
	vout.PrevPosH = mul(float4(PosLBlend.xyz, 1.0f), gPrevWorldViewProj);
	vout.NormalV = mul(NormalLBlend, gWorldViewInvTranspose);
	vout.TangentV = mul(TangentLBlend, gWorldViewInvTranspose);
	vout.TexUV = vin.TexUV;
	vout.ShadowPosH = mul(float4(PosLBlend.xyz, 1.0f), gShadowTransform);
	vout.PosW = mul(float4(PosLBlend.xyz, 1.0f), gWorld);
	vout.NormalW = mul(NormalLBlend, gWorldInvTranspose);
	//vout.PosV = mul(float4(vin.PosL, 1.0f), gWorldView);
    return vout;
}