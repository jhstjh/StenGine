cbuffer cbPerObject: register(b0) {
	//float4x4 gWorld;
	//float4x4 gViewProj;
	float4x4 gWorldViewProj;
	//float4x4 gTexTranfrom;
};

// don't need it if we don't use alpha clip
//Texture2D gDiffuseMap;

struct VertexIn {
	float3 PosL : POSITION;
};

struct VertexOut {
	float4 PosH : SV_POSITION;
};

VertexOut main(VertexIn vin) {
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	//vout.PosH.z = 0;
	return vout;
}