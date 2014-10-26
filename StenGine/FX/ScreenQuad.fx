struct DirectionalLight {
	float4 intensity;
	float3 direction;
	float pad;
};

cbuffer cbPerFrame {
	DirectionalLight gDirLight;
	float3 gEyePosW;
	float4x4 gProjInv;
	float4x4 gProj;

	float    gOcclusionRadius = 0.2f;
	float    gOcclusionFadeStart = 0.1f;
	float    gOcclusionFadeEnd = 2.0f;
	float    gSurfaceEpsilon = 0.005f;
};

struct VSOut
{
	float4 pos : SV_Position;
	float2 Tex : TEXCOORD;
};

struct PSOut
{
	float4 DeferredShade: SV_TARGET0;
	float4 SSAO: SV_TARGET1;
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 1e5f);
};


cbuffer cbSettings
{
	float gWeights[21] =
	{
		0.013757049563821119, 0.01888216932766378, 0.025066978648142262, 0.03218663770378345, 0.03997355278034706, 0.048016821060535536, 0.05578758693508665, 0.06269100992275224, 0.06813911255361485, 0.07163267956032793, 0.07283656203947195, 0.07163267956032793, 0.06813911255361485, 0.06269100992275224, 0.05578758693508665, 0.048016821060535536, 0.03997355278034706, 0.03218663770378345, 0.025066978648142262, 0.01888216932766378, 0.013757049563821119,
	};
};

cbuffer cbFixed
{
	static const int gBlurRadius = 10;
};

Texture2D gScreenMap;
Texture2D gSSAOMap;

Texture2D gDiffuseGB;
Texture2D gPositionGB;
Texture2D gNormalGB;
Texture2D gSpecularGB;
Texture2D gDepthGB;

VSOut VSmain(uint vertexId : SV_VertexID)
{
	VSOut output;

	if (vertexId == 0) {
		output.pos = float4(-1.0, -1.0, 0.0, 1.0);
		output.Tex = float2(0, 1);
	}
	else if (vertexId == 1) {
		output.pos = float4(-1.0, 1.0, 0.0, 1.0);
		output.Tex = float2(0, 0);
	}
	else if (vertexId == 2) {
		output.pos = float4(1.0, 1.0, 0.0, 1.0);
		output.Tex = float2(1, 0);
	}
	else if (vertexId == 3) {
		output.pos = float4(1.0, 1.0, 0.0, 1.0);
		output.Tex = float2(1, 0);
	}
	else if (vertexId == 4) {
		output.pos = float4(1.0, -1.0, 0.0, 1.0);
		output.Tex = float2(1, 1);
	}
	else if (vertexId == 5) {
		output.pos = float4(-1.0, -1.0, 0.0, 1.0);
		output.Tex = float2(0, 1);
	}
	
	return output;
}



struct PSIn
{
	float4 pos : SV_Position;
	linear float2 Tex : TEXCOORD;
};


float4 OffsetVect[] = { 
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

float4 PSSSAOmain(PSIn input) : SV_Target
{
	float z = gDepthGB.Sample(samNormalDepth, input.Tex);// tex2D(DepthSampler, vTexCoord);

	float x = input.Tex.x * 2 - 1;
	float y = (1 - input.Tex.y) * 2 - 1;
	float4 vProjectedPos = float4(x, y, z, 1.0f);
		// Transform by the inverse projection matrix
		float4 vPositionVS = mul(vProjectedPos, gProjInv);
		// Divide by w to get the view-space position
		vPositionVS /= vPositionVS.w;

	float3 normalV;
	normalV.xy = gNormalGB.Sample(samNormalDepth, input.Tex).xy;
	normalV.z = -sqrt(1 - dot(normalV.xy, normalV.xy));


	float occlusionSum = 0.0f;

	for (int i = 0; i < 14; ++i)
	{
		float3 offset = normalize(OffsetVect[i].xyz);

			float flip = 1.0;//sign(dot(offset, normalV));

		float3 qV = vPositionVS.xyz + flip * gOcclusionRadius * offset;

			float4 projQ = mul(float4(qV, 1.0f), gProj);
			projQ /= projQ.w;

		float rz = gDepthGB.Sample(samNormalDepth, float2(0.5 * projQ.x + 0.5, 0.5 - 0.5*projQ.y));
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
	return saturate(pow(access, 4.0f));
}

float4 PSBlurmain(PSIn input, uniform float2 texOffset) : SV_Target
{
	// The center value always contributes to the sum.
	float4 color = gWeights[10] * gSSAOMap.Sample(samAnisotropic, input.Tex);
	float totalWeight = gWeights[10];

// 	float centerDepth = gDepthGB.Sample(samAnisotropic, input.Tex);
// 
// 	float x = input.Tex.x * 2 - 1;
// 	float y = (1 - input.Tex.y) * 2 - 1;
// 	float4 vProjectedPos = float4(x, y, centerDepth, 1.0f);
// 	float4 vPositionVS = mul(vProjectedPos, gProjInv);
// 	vPositionVS.xyz /= vPositionVS.w;
// 
// 	float4 centerNormal = gNormalGB.Sample(samAnisotropic, input.Tex);

	for (float i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		// We already added in the center weight.
		if (i == 0)
			continue;

		float2 tex = input.Tex + i*texOffset;

// 		float neighborDepth = gDepthGB.Sample(samAnisotropic, tex);
// 		float4 neighborNormal = gNormalGB.Sample(samAnisotropic, tex);
// 
// 		float x = input.Tex.x * 2 - 1;
// 		float y = (1 - input.Tex.y) * 2 - 1;
// 		float4 nProjectedPos = float4(x, y, neighborDepth, 1.0f);
// 		float4 nPositionVS = mul(vProjectedPos, gProjInv);
// 		nPositionVS.xyz /= nPositionVS.w;

// 		if (dot(neighborNormal.xyz, centerNormal.xyz) >= 0.8f &&
//			abs(nPositionVS.z - vPositionVS.z) <= 0.2f)
		{
			float weight = gWeights[i + gBlurRadius];

			// Add neighbor pixel to blur.
			color += weight * gSSAOMap.Sample(samAnisotropic, tex);

			totalWeight += weight;
		}
	}
	
	// Compensate for discarded samples by making total weights sum to 1.
	color /= totalWeight;
	if (texOffset.x > 0) return saturate(color) * gScreenMap.Sample(samAnisotropic, input.Tex);
	else return color;
}

PSOut PSmain(PSIn input) 
{
	//---------------------- below for screen composition -----------------------//
	PSOut pOut;

	float z = gDepthGB.Sample(samAnisotropic, input.Tex);// tex2D(DepthSampler, vTexCoord);

	float x = input.Tex.x * 2 - 1;
	float y = (1 - input.Tex.y) * 2 - 1;
	float4 vProjectedPos = float4(x, y, z, 1.0f);
		// Transform by the inverse projection matrix
	float4 vPositionVS = mul(vProjectedPos, gProjInv);
		// Divide by w to get the view-space position
	vPositionVS.xyz /= vPositionVS.w;

	//return vPositionVS;
	//return gPositionGB.Sample(samAnisotropic, input.Tex);

	clip(1000 - vPositionVS.z - 1);
		

	float4 diffColor = float4(0, 0, 0, 0);
	float4 specColor = float4(0, 0, 0, 0);

	float3 normalV;
	normalV.xy = gNormalGB.Sample(samAnisotropic, input.Tex).xy;
	normalV.z = - sqrt(1 - dot(normalV.xy, normalV.xy));



	float diffuseK = dot(-gDirLight.direction, normalV);
	float shadowLit = gDiffuseGB.Sample(samAnisotropic, input.Tex).w;

	if (diffuseK > 0) {
		diffColor += diffuseK * gDirLight.intensity;
		float3 refLight = reflect(gDirLight.direction, normalV);
		float3 viewRay = gEyePosW - vPositionVS.xyz;
		viewRay = normalize(viewRay);
		specColor += gSpecularGB.Sample(samAnisotropic, input.Tex) * pow(max(dot(refLight, viewRay), 0), gSpecularGB.Sample(samAnisotropic, input.Tex).w * 255.0f);
	}

	pOut.DeferredShade = (float4(0.2, 0.2, 0.2, 0) + diffColor * shadowLit) * gDiffuseGB.Sample(samAnisotropic, input.Tex) + specColor * shadowLit /*+ float4(1, 1, 1, 1) * vPositionVS.z / 20*/;
	
	// ---------------- SSAO ---------------//

	float occlusionSum = 0.0f;

	for (int i = 0; i < 14; ++i)
	{
		float3 offset = normalize(OffsetVect[i].xyz);

		float flip = 1.0;//sign(dot(offset, normalV));

		float3 qV = vPositionVS.xyz + flip * gOcclusionRadius * offset;

		float4 projQ = mul(float4(qV, 1.0f), gProj);
		projQ /= projQ.w;

		float rz = gDepthGB.Sample(samNormalDepth, float2(0.5 * projQ.x + 0.5, 0.5 - 0.5*projQ.y));
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

	return pOut;
}


technique11 t0
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, VSmain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSmain()));
		SetBlendState(0, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}

technique11 DeferredLightingTech
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, VSmain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSmain()));
	}
}

technique11 SSAOTech
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, VSmain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSSSAOmain()));
		SetBlendState(0, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}

technique11 HBlurTech
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, VSmain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSBlurmain(float2(1.0/1280, 0.0f))));
		SetBlendState(0, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}

technique11 VBlurTech
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, VSmain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSBlurmain(float2(0.0f, 1.0/720))));
		SetBlendState(0, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}