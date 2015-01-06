struct VSOut
{
	float4 PosH : SV_Position;
	float3 PosL : POSITION;
};

TextureCube gCubeMap;
SamplerState gSamplerStateLinear;

float4 main(VSOut pin): SV_Target
{
	//pin.PosH /= pin.PosH.w;
	return gCubeMap.Sample(gSamplerStateLinear, pin.PosL) * 1.5;
}