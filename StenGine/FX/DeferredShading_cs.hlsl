struct DirectionalLight {
	float4 intensity;
	float3 direction;
	float pad;
};

cbuffer cbPerFrame: register(b0) {
	DirectionalLight gDirLight;
	float3 gEyePosW;
	float4x4 gProjInv;
	float4x4 gProj;
};

Texture2D gDiffuseGB: register(t0);
Texture2D gNormalGB: register(t1);
Texture2D gSpecularGB: register(t2);
Texture2D gDepthGB: register(t3);

RWTexture2D<float4> gOutput: register(u0);

SamplerState gLinearSampler: register(s0);

#define N 16

[numthreads(N, N, 1)]
void main(int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID)
{
	//float z = gDepthGB.Load(uint3(dispatchThreadID.xy, 0)).r;// tex2D(DepthSampler, vTexCoord);
	float z = gDepthGB[dispatchThreadID.xy].r;

	float x = dispatchThreadID.x/1280.f * 2 - 1;
	float y = (1 - dispatchThreadID.y/720.f) * 2 - 1;
	float4 vProjectedPos = float4(x, y, z, 1.0f);
	float4 vPositionVS = mul(vProjectedPos, gProjInv);
	vPositionVS.xyz /= vPositionVS.w;

	//return vPositionVS;
	//return gPositionGB.Sample(samAnisotropic, input.Tex);

	//if (1000 - vPositionVS.z - 1 < 0) return;


	float4 diffColor = float4(0, 0, 0, 0);
	float4 specColor = float4(0, 0, 0, 0);

	float3 normalV;
	normalV.xy = gNormalGB[dispatchThreadID.xy].xy;
	normalV.z = -sqrt(1 - dot(normalV.xy, normalV.xy));



	float diffuseK = dot(-gDirLight.direction, normalV);
	float shadowLit = /*1; */gDiffuseGB[dispatchThreadID.xy].w;

	if (diffuseK > 0) {
		diffColor += diffuseK * gDirLight.intensity;
		float3 refLight = reflect(gDirLight.direction, normalV);
		float3 viewRay = gEyePosW - vPositionVS.xyz;
		viewRay = normalize(viewRay);
		specColor += gSpecularGB[dispatchThreadID.xy] * pow(max(dot(refLight, viewRay), 0), gSpecularGB[dispatchThreadID.xy].w * 255.0f);
	}

	gOutput[dispatchThreadID.xy] = (float4(0.2, 0.2, 0.2, 0) + diffColor * shadowLit) * gDiffuseGB[dispatchThreadID.xy] + specColor * shadowLit /*+ float4(1, 1, 1, 1) * vPositionVS.z / 20*/;;
}