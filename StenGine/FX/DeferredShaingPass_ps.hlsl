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

cbuffer cbFixed {
	half4 vertexArray[6] = {
		half4(-1.0, -1.0, 0.0, 1.0),
		half4(-1.0, 1.0, 0.0, 1.0),
		half4(1.0, 1.0, 0.0, 1.0),
		half4(1.0, 1.0, 0.0, 1.0),
		half4(1.0, -1.0, 0.0, 1.0),
		half4(-1.0, -1.0, 0.0, 1.0),
	};

	half2 uvArray[6] = {
		half2(0, 1),
		half2(0, 0),
		half2(1, 0),
		half2(1, 0),
		half2(1, 1),
		half2(0, 1),
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
};

struct PSOut
{
	float4 DeferredShade: SV_TARGET0;
	//float4 SSAO: SV_TARGET1;
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

struct PSIn
{
	float4 pos : SV_Position;
	linear float2 Tex : TEXCOORD;
};


PSOut main(PSIn input) 
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

// 	float occlusionSum = 0.0f;
// 
// 	for (int i = 0; i < 14; ++i)
// 	{
// 		float3 offset = normalize(OffsetVect[i].xyz);
// 
// 		float flip = 1.0;//sign(dot(offset, normalV));
// 
// 		float3 qV = vPositionVS.xyz + flip * gOcclusionRadius * offset;
// 
// 		float4 projQ = mul(float4(qV, 1.0f), gProj);
// 		projQ /= projQ.w;
// 
// 		float rz = gDepthGB.Sample(samNormalDepth, float2(0.5 * projQ.x + 0.5, 0.5 - 0.5*projQ.y));
// 		float4 rProjectedPos = float4(projQ.x, projQ.y, rz, 1.0f);
// 		float4 rPositionVS = mul(rProjectedPos, gProjInv);
// 		rPositionVS /= rPositionVS.w;
// 
// 
// 		float distZ = vPositionVS.z - rPositionVS.z;
// 		float dp = max(dot(normalV, normalize(rPositionVS.xyz - vPositionVS.xyz)), 0.0f);
// 		float occlusion = dp * OcclusionFunction(distZ);
// 
// 		occlusionSum += occlusion;
// 	}
// 
// 	occlusionSum /= 14.0f;
// 
// 	float access = 1.0f - occlusionSum;
// 
// 	// Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
// 	pOut.SSAO =  saturate(pow(access, 4.0f));

	return pOut;
}

