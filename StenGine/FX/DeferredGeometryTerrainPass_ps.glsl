#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_EXT_texture_array : enable

struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 roughness_metalic_c_doublesided;
};

struct DirectionalLight {
	vec4 intensity;
	vec3 direction;
	float pad;
};

in TesOut {
	vec4 PosH;
	vec3 PosW;
	vec2 TexUV;
	vec2 TiledTex;
	vec4 ShadowPosH;
} pIn;

//out vec4 ps_color;
layout(location = 0) out vec4 ps_norm;
layout(location = 1) out vec4 ps_diff;
layout(location = 2) out vec4 ps_spec;


layout(std140) uniform ubPerObj {
	mat4 gWorldViewProj;
	mat4 gWorldViewInvTranspose;
	mat4 gWorldInvTranspose;
	mat4 gWorldView;
	mat4 gWorld;
	mat4 gViewProj;
	mat4 gShadowTransform;
	mat4 gView;

	Material gMat;
	vec4 DiffX_NormY_ShadZ;
};

layout(std140) uniform ubTextures {	
	sampler2D		gShadowMap;
	samplerCube		gCubeMap;
	sampler2D		gHeightMap;
	sampler2DArray	gLayerMapArray;
	sampler2D		gBlendMap;
};

layout(std140) uniform ubPerFrame{
	vec3 gEyePosW;

	float pad0;

	// When distance is minimum, the tessellation is maximum.
	// When distance is maximum, the tessellation is minimum.
	float gMinDist;
	float gMaxDist;

	// Exponents for power of 2 tessellation.  The tessellation
	// range is [2^(gMinTess), 2^(gMaxTess)].  Since the maximum
	// tessellation is 64, this means gMaxTess can be at most 6
	// since 2^6 = 64.
	float gMinTess;
	float gMaxTess;

	float gTexelCellSpaceU;
	float gTexelCellSpaceV;
	float gWorldCellSpace;

	float pad1;
	vec2 gTexScale/* = vec2(50.0f)*/;

	vec2 pad2;
	vec4 gWorldFrustumPlanes[6];
};

void main() {
	vec2 leftTex = pIn.TexUV + vec2(-gTexelCellSpaceU, 0.0f);
	vec2 rightTex = pIn.TexUV + vec2(gTexelCellSpaceU, 0.0f);
	vec2 bottomTex = pIn.TexUV + vec2(0.0f, gTexelCellSpaceV);
	vec2 topTex = pIn.TexUV + vec2(0.0f, -gTexelCellSpaceV);

	float leftY = textureLod(gHeightMap, leftTex, 0).r;
	float rightY = textureLod(gHeightMap, rightTex, 0).r;
	float bottomY = textureLod(gHeightMap, bottomTex, 0).r;
	float topY = textureLod(gHeightMap, topTex, 0).r;

	vec3 tangent = normalize(vec3(2.f * gWorldCellSpace, rightY - leftY, 0.f));
	vec3 bitan = normalize(vec3(0.f, bottomY - topY, -2.f * gWorldCellSpace));
	vec3 normalW = cross(tangent, bitan);

	ps_norm = gView * vec4(normalW, 0.f);
	ps_norm.w = 1.0f;

	ps_norm = (ps_norm + 1) / 2.0f;

	vec4 c0 = texture2DArray(gLayerMapArray, vec3(pIn.TiledTex, 0.f));
	vec4 c1 = texture2DArray(gLayerMapArray, vec3(pIn.TiledTex, 1.f));
	vec4 c2 = texture2DArray(gLayerMapArray, vec3(pIn.TiledTex, 2.f));
	vec4 c3 = texture2DArray(gLayerMapArray, vec3(pIn.TiledTex, 3.f));
	vec4 c4 = texture2DArray(gLayerMapArray, vec3(pIn.TiledTex, 4.f));

	vec4 t = texture2D(gBlendMap, pIn.TexUV);

	vec4 texColor = c0;
	texColor = mix(texColor, c1, t.r);
	texColor = mix(texColor, c2, t.g);
	texColor = mix(texColor, c3, t.b);
	texColor = mix(texColor, c4, t.a);

	ps_diff = texColor;
	ps_diff.w = 1.0;
	ps_spec = vec4(0, 0, 0, 0);

	vec4 shadowTrans = pIn.ShadowPosH;

	shadowTrans.xyz /= shadowTrans.w;

	if (!(shadowTrans.x < 0 || shadowTrans.x > 1 || shadowTrans.y < 0 || shadowTrans.y > 1))
	{
		//if (DiffX_NormY_ShadZ.z > 0)
		{
			float epsilon = 0.003;
			float shadow = texture2D(gShadowMap, shadowTrans.xy).r;
			shadow += epsilon;
			if (shadow  < shadowTrans.z) {
				ps_diff.w = 0.0;
			}
		}
	}
}