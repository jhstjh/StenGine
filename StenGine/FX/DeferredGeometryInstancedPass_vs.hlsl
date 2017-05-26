// struct DirectionalLight {
// 	float4 intensity;
// 	float3 direction;
// 	float pad;
// };

#include "StandardConstant.fx"

VertexOut main(VertexIntancedIn vin)
{
	VertexOut vout;

    vin.PosL.x += vin.instancePosition.x;
    vin.PosL.y += vin.instancePosition.y;
    vin.PosL.z += vin.instancePosition.z;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.NormalV = mul(vin.NormalL, gWorldViewInvTranspose);
	vout.TangentV = mul(vin.TangentL, gWorldViewInvTranspose);
	vout.TexUV = vin.TexUV;
	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.NormalW = mul(vin.NormalL, gWorldInvTranspose);
	//vout.PosV = mul(float4(vin.PosL, 1.0f), gWorldView);
    return vout;
}