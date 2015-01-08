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
Texture2D gBumpMap: register(t4);

SamplerState gSamplerStateLinear;

cbuffer cbFixed: register(b13) {
	static const float    gOcclusionRadius = 0.08f;
	static const float    gOcclusionFadeStart = 0.1f;
	static const float    gOcclusionFadeEnd = 8.0f;
	static const float    gSurfaceEpsilon = 0.0005f;

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

struct PSOut
{
	float4 DeferredShade: SV_TARGET0;
	float4 SSAO: SV_TARGET1;
};


struct PSIn
{
	float4 pos : SV_Position;
	linear float2 Tex : TEXCOORD;
};

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
	//						 1.0     -------------\
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


PSOut main(PSIn input) 
{
	//---------------------- below for screen composition -----------------------//
	PSOut pOut;

	float z = gDepthGB.Sample(gSamplerStateLinear, input.Tex);// tex2D(DepthSampler, vTexCoord);

	float x = input.Tex.x * 2 - 1;
	float y = (1 - input.Tex.y) * 2 - 1;
	float4 vProjectedPos = float4(x, y, z, 1.0f);
	float4 vPositionVS = mul(vProjectedPos, gProjInv);
	vPositionVS.xyz /= vPositionVS.w;

	//return vPositionVS;
	//return gPositionGB.Sample(samAnisotropic, input.Tex);

	clip(1000 - vPositionVS.z - 1);
		

//	float4 diffColor = float4(0, 0, 0, 0);
//	float4 specColor = float4(0, 0, 0, 0);
//
	float3 normalV;
	//normalV.xy = gNormalGB.Sample(gSamplerStateLinear, input.Tex).xy;
	//normalV.z = - sqrt(1 - dot(normalV.xy, normalV.xy));
	normalV = gNormalGB.Sample(gSamplerStateLinear, input.Tex).xyz * 2 - 1;
//
//
//
	float diffuseK = dot(-gDirLight.direction, normalV);
	float shadowLit = /*1; */gDiffuseGB.Sample(gSamplerStateLinear, input.Tex).w;
//
//	if (diffuseK > 0) {
//		diffColor += diffuseK * gDirLight.intensity;
//		float3 refLight = reflect(gDirLight.direction, normalV);
//		float3 viewRay = gEyePosW - vPositionVS.xyz;
//		viewRay = normalize(viewRay);
//		specColor += gSpecularGB.Sample(gSamplerStateLinear, input.Tex) * pow(max(dot(refLight, viewRay), 0), gSpecularGB.Sample(gSamplerStateLinear, input.Tex).w * 255.0f);
//	}
//
//	pOut.DeferredShade = (float4(0.2, 0.2, 0.2, 0) + diffColor * shadowLit) * gDiffuseGB.Sample(gSamplerStateLinear, input.Tex) + specColor * shadowLit /*+ float4(1, 1, 1, 1) * vPositionVS.z / 20*/;
	
	float roughnessFactor = 0.15;

	float3 viewRay = normalize(float3(0, 0, 0) - vPositionVS.xyz);
	float3 light = -gDirLight.direction;

	float3 halfVec = normalize(light + viewRay);
	float NdotL = clamp(dot(normalV, light), 0.0, 1.0);
	float NdotH = clamp(dot(normalV, halfVec), 0.0, 1.0);
	float NdotV = clamp(dot(normalV, viewRay), 0.0, 1.0);
	float VdotH = clamp(dot(viewRay, halfVec), 0.0, 1.0);
	float LdotH = clamp(dot(light, halfVec), 0.0, 1.0);
	float r_sq = roughnessFactor * roughnessFactor;


	float geoNumerator = 2.0f * NdotH;
	float geoDenominator = VdotH;

	float geoB = (geoNumerator * NdotV) / VdotH;
	float geoC = (geoNumerator * NdotL) / LdotH;
	float geo = min(1.0, min(geoB, geoC));


	float roughness;

	//	float roughness_a = 1.0f / (4.0f * r_sq * pow(NdotH, 4));
	//	float roughness_b = NdotH * NdotH - 1.0;
	//	float roughness_c = r_sq * NdotH * NdotH;

	//	roughness = roughness_a * exp(roughness_b / roughness_c);
	float c = 1.0f;
	float alpha = acos(dot(normalV, halfVec));
	roughness = c * exp(-(alpha / r_sq));


	roughness = 1.0 / (3.1415926 * r_sq * pow(NdotH, 4)) * exp((NdotH * NdotH - 1) / (r_sq * NdotH * NdotH));



	float ref_at_norm_incidence = 0.8;
	float fresnel = pow(1.0 - VdotH, 5.0f);
	fresnel *= (1.0f - ref_at_norm_incidence);
	fresnel += ref_at_norm_incidence;

	float rsnum1d = fresnel * geo * roughness;
	float3 Rs_numerator = float3(rsnum1d, rsnum1d, rsnum1d);
	float Rs_denominator = NdotV * NdotL * 3.1415926;
	float3 Rs = Rs_numerator / Rs_denominator;

	float3 diffuseFactor = gDiffuseGB.Sample(gSamplerStateLinear, input.Tex).xyz;
	float3 specularFactor = gSpecularGB.Sample(gSamplerStateLinear, input.Tex).xyz;

	float3 cDiffuse = diffuseFactor * ((max(0, diffuseK) * gDirLight.intensity.xyz) * shadowLit);


	float3 final = max(0.0f, NdotL) * (clamp(specularFactor * Rs * shadowLit, 0.0, 1.0) + cDiffuse) + float3(0.2, 0.2, 0.2) * diffuseFactor;

	//float3 r = float3(0.2, 0.2, 0.2) + NdotL * (0.8 + (1 - 0.8) * Rs);

	//ps_color = float4(r * diffuseFactor, 1);
	pOut.DeferredShade = float4(final, 1.0);

	// ---------------- SSAO ---------------//

#if 0	
	float occlusionSum = 0.0f;

	for (int i = 0; i < 14; ++i)
	{
		float3 offset = normalize(OffsetVect[i].xyz);

		float flip = 1.0;//sign(dot(offset, normalV));

		float3 qV = vPositionVS.xyz + flip * gOcclusionRadius * offset;

		float4 projQ = mul(float4(qV, 1.0f), gProj);
		projQ /= projQ.w;

		float rz = gDepthGB.Sample(gSamplerStateLinear, float2(0.5 * projQ.x + 0.5, 0.5 - 0.5*projQ.y));
		float4 rProjectedPos = float4(projQ.x, projQ.y, rz, 1.0f);
		float4 rPositionVS = mul(rProjectedPos, gProjInv);
		rPositionVS /= rPositionVS.w;


		float distZ = vPositionVS.z - rPositionVS.z;
		float dp = max(dot(normalV, normalize(rPositionVS.xyz - vPositionVS.xyz)), 0.0f);
		float occlusion = dp * OcclusionFunction(distZ);

		occlusionSum += occlusion;
	}

	occlusionSum /= 14.0f;

	float access = 1.0f - occlusionSum;

	// Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
	pOut.SSAO =  saturate(pow(access, 4.0f));

#else
	//float3 randVect =  2 * sample2D(gBumpMapSampler, 16 * pIn.iColor.xy, gBumpMap) - 1;
	float3 randVect = normalize(gBumpMap.SampleLevel(gSamplerStateLinear, 16 * input.Tex, 0));

	float occlusionSum = 0.0f;
	float4 occlusionColor = float4(0, 0, 0, 0);
	for (int i = 0; i < 14; ++i)
	{
		float3 offset = reflect(normalize(OffsetVect[i].xyz), randVect);

		float flip = 1;//sign(dot(offset, normal));

		float3 qV = vPositionVS.xyz + flip * gOcclusionRadius * offset;

		float4 projQ = mul(float4(qV, 1.0f), gProj);
		projQ /= projQ.w;

		float rz = gDepthGB.SampleLevel(gSamplerStateLinear, float2(0.5 * projQ.x + 0.5, 0.5 - 0.5 * projQ.y), 0).r;
		float4 rProjectedPos = float4(projQ.x, projQ.y, rz, 1.0f);
		float4 rPositionVS = mul(rProjectedPos, gProjInv);
		rPositionVS /= rPositionVS.w;

		float3 diff = rPositionVS.xyz - vPositionVS;
		const float3 v = normalize(diff);
		const float d = length(diff)*0.05;
		float occlusion = max(0.0, dot(normalV, v) - gSurfaceEpsilon) * (1.0 / (1.0 + d)) * 0.8;
		occlusionSum += occlusion;
	}

	occlusionSum /= 14.0f;

	float access = 1.0f - occlusionSum;
	pOut.SSAO = saturate(pow(access, 4.0f));
#endif
	return pOut;
}

