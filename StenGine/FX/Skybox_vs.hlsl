cbuffer cbPerObject: register(b0) {
	float4x4 gWorldViewProj;
};

cbuffer cbFixed: register(b13) {
	static const float3 vertexArray[24] = {
		float3(-1.0f, -1.0f, -1.0f),
		float3(-1.0f, +1.0f, -1.0f),
		float3(+1.0f, +1.0f, -1.0f),
		float3(+1.0f, -1.0f, -1.0f),
		float3(-1.0f, -1.0f, +1.0f),
		float3(+1.0f, -1.0f, +1.0f),
		float3(+1.0f, +1.0f, +1.0f),
		float3(-1.0f, +1.0f, +1.0f),
		float3(-1.0f, +1.0f, -1.0f),
		float3(-1.0f, +1.0f, +1.0f),
		float3(+1.0f, +1.0f, +1.0f),
		float3(+1.0f, +1.0f, -1.0f),
		float3(-1.0f, -1.0f, -1.0f),
		float3(+1.0f, -1.0f, -1.0f),
		float3(+1.0f, -1.0f, +1.0f),
		float3(-1.0f, -1.0f, +1.0f),
		float3(-1.0f, -1.0f, +1.0f),
		float3(-1.0f, +1.0f, +1.0f),
		float3(-1.0f, +1.0f, -1.0f),
		float3(-1.0f, -1.0f, -1.0f),
		float3(+1.0f, -1.0f, -1.0f),
		float3(+1.0f, +1.0f, -1.0f),
		float3(+1.0f, +1.0f, +1.0f),
		float3(+1.0f, -1.0f, +1.0f),
	};

	static const uint indexArray[36] = {

		1, 0, 2,
		2, 0, 3,

		5, 4, 6,
		6, 4, 7,

		9, 8, 10,
		10, 8, 11,

		13, 12, 14,
		14, 12, 15,

		17, 16, 18,
		18, 16, 19,

		21, 20, 22,
		22, 20, 23
	};
};

struct VSOut
{
	float4 PosH : SV_Position;
	float3 PosL : POSITION;
};

SamplerState samTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};


VSOut main(uint vertexId : SV_VertexID)
{
	VSOut output;

	output.PosL = vertexArray[indexArray[vertexId]];

	output.PosH = mul(float4(output.PosL, 1.0f), gWorldViewProj).xyww;
	//output.PosH /= output.PosH.w;
	output.PosH.z -= 0.001;

	return output;
}