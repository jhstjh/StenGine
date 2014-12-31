#define PRIMITIVE_LINE
#include "StandardConstant.fx"

DebugLineVertexOut main(DebugLineVertexIn vIn, uint vertexId : SV_VertexID) {
	DebugLineVertexOut vOut;

	//vOut.Color = vIn.Color;
	vOut.PosH = mul(float4(vIn.PosW, 1.0f), gViewProj);

	if (vertexId < 2) vOut.Color = float4(1, 0, 0, 1);
	else if (vertexId < 4) vOut.Color = float4(0, 1, 0, 1);
	else if (vertexId < 6) vOut.Color = float4(0, 0, 1, 1);
	else vOut.Color = float4(0.5, 0.5, 0.5, 1);

	return vOut;
}