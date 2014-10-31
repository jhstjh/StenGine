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

struct VSOut
{
	float4 pos : SV_Position;
	float2 Tex : TEXCOORD;
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

//Texture2D gScreenMap;
//Texture2D gSSAOMap;

Texture2D gDiffuseGB;
//Texture2D gPositionGB;
Texture2D gNormalGB;
Texture2D gSpecularGB;
Texture2D gDepthGB;

VSOut main(uint vertexId : SV_VertexID)
{
	VSOut output;

	output.pos = vertexArray[vertexId];
	output.Tex = uvArray[vertexId];

	return output;
}
