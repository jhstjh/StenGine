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

cbuffer cbFixed: register(b13) {
	static const float    gOcclusionRadius = 0.2f;
	static const float    gOcclusionFadeStart = 0.1f;
	static const float    gOcclusionFadeEnd = 2.0f;
	static const float    gSurfaceEpsilon = 0.005f;

	static const float4 OffsetVect[] = {
			{ +1.0f, +1.0f, +1.0f, 0.0f },
			{ -1.0f, -1.0f, -1.0f, 0.0f },
			{ -1.0f, +1.0f, +1.0f, 0.0f },
			{ +1.0f, -1.0f, -1.0f, 0.0f },
			{ +1.0f, +1.0f, -1.0f, 0.0f },
			{ -1.0f, -1.0f, +1.0f, 0.0f },
			{ -1.0f, +1.0f, -1.0f, 0.0f },
			{ +1.0f, -1.0f, +1.0f, 0.0f },
			{ -1.0f, 0.0f, 0.0f, 0.0f },
			{ +1.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, -1.0f, 0.0f, 0.0f },
			{ 0.0f, +1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, -1.0f, 0.0f },
			{ 0.0f, 0.0f, +1.0f, 0.0f },
	};

	static const int gBlurRadius = 10;
};

Texture2D gDiffuseGB: register(t0);
Texture2D gNormalGB: register(t1);
Texture2D gSpecularGB: register(t2);
Texture2D gDepthGB: register(t3);

RWTexture2D<float4> gDeferredShaded: register(u0);
RWTexture2D<float4> gSSAO: register(u1);

SamplerState gLinearSampler: register(s0);

#define N 16

float OcclusionFunction(float distZ)
{
	//
	// If depth(q) is "behind" depth(p), then q cannot occlude p.  Moreover, if 
	// depth(q) and depth(p) are sufficiently close, then we also assume q cannot
	// occlude p because q needs to be in front of p by Epsilon to occlude p.
	//
	// We use the following function to determine the occlusion.  
	// 
	//
	//               1.0     -------------\
							//               |           |  \
							//               |           |    \
							//               |           |      \ 
	//               |           |        \
							//               |           |          \
							//               |           |            \
							//  ------|------|-----------|-------------|---------|--> zv
	//        0     Eps          z0            z1        
	//

	float occlusion = 0.0f;
	if (distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;

		// Linearly decrease occlusion from 1 to 0 as distZ goes 
		// from gOcclusionFadeStart to gOcclusionFadeEnd.	
		occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
	}

	return occlusion;
}

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
 
 	gDeferredShaded[dispatchThreadID.xy] = (float4(0.2, 0.2, 0.2, 0) + diffColor * shadowLit) * gDiffuseGB[dispatchThreadID.xy] + specColor * shadowLit /*+ float4(1, 1, 1, 1) * vPositionVS.z / 20*/;;

	// ---------------- SSAO ---------------//

	float occlusionSum = 0.0f;
	int sampleCount = 0;
	for (int i = 0; i < 14; ++i)
	{
		float3 offset = normalize(OffsetVect[i].xyz);

		float flip = 1.0;//sign(dot(offset, normalV));

		float3 qV = vPositionVS.xyz + flip * gOcclusionRadius * offset;

		float4 projQ = mul(float4(qV, 1.0f), gProj);
		projQ /= projQ.w;

		//if (projQ.x > 1 || projQ.x < -1 || projQ.y > 1 || projQ.y < -1) continue;

		//float rz = gDepthGB[float2((projQ.x)*1280, (projQ.y))*720].r;
		float rz = gDepthGB.SampleLevel(gLinearSampler, float2((0.5 * projQ.x + 0.5), (0.5 - 0.5*projQ.y)), 0).r;
		float4 rProjectedPos = float4(projQ.x, projQ.y, rz, 1.0f);
		float4 rPositionVS = mul(rProjectedPos, gProjInv);
		rPositionVS /= rPositionVS.w;


		float distZ = vPositionVS.z - rPositionVS.z;
		float dp = max(dot(normalV, normalize(rPositionVS.xyz - vPositionVS.xyz)), 0.0f);
		float occlusion = dp * OcclusionFunction(distZ);

		occlusionSum += occlusion;
		sampleCount++;
		//gSSAO[dispatchThreadID.xy] = projQ;
	}

	occlusionSum /= sampleCount;

	float access = 1.0f - occlusionSum;

	// Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
	gSSAO[dispatchThreadID.xy] = saturate(pow(access, 4.f));
	
}