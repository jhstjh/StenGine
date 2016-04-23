#version 450
#extension GL_ARB_bindless_texture : require

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

in VertexOut {
	vec2 pTexUV;
	vec3 pNormalV;
	vec3 pNormalW;
	vec3 pPosW;
	vec3 pTangV;
	vec4 pShadowTransform;
} pIn;

//out vec4 ps_color;
layout(location = 0) out vec4 ps_norm;
layout(location = 1) out vec4 ps_diff;
layout(location = 2) out vec4 ps_spec;


layout(std140) uniform ubPerObj{
	mat4 gWorldViewProj;
	mat4 gWorld;
	mat4 gWorldView;
	mat4 gViewProj;
	mat4 gShadowTransform;
	Material gMat;
	vec4 DiffX_NormY_ShadZ;

	sampler2D gDiffuseMap;
	sampler2D gNormalMap;
	sampler2D gShadowMap;
	sampler2D gBumpMap;
	samplerCube gCubeMap;
};

layout(std140) uniform ubPerFrame{
	vec4 gEyePosW;
	DirectionalLight gDirLight;
};

void main() {
	vec3 normal = normalize(pIn.pNormalV);

	if (DiffX_NormY_ShadZ.y > 0) {
		vec3 normalMapNormal = texture(gNormalMap, pIn.pTexUV).xyz;
		normalMapNormal = 2.0f * normalMapNormal - 1.0;

		vec3 N = normalize(normal);
		vec3 T = normalize(pIn.pTangV - dot(pIn.pTangV, N)*N);
		vec3 B = cross(N, T);

		mat3 TBN = mat3(T, B, N);

		normal = normalize(TBN * normalMapNormal);//normalize(pin.NormalV).xy;
	}

	ps_norm = (vec4(normal, 1.0) + 1) / 2.0f;
	ps_norm.w = 1.0f;

	vec3 eyeRay = normalize(pIn.pPosW - gEyePosW.xyz);
	vec3 refRay = reflect(eyeRay, pIn.pNormalW);

	ps_diff = ((1 - DiffX_NormY_ShadZ.x) * textureLod(gCubeMap, refRay, 3) + DiffX_NormY_ShadZ.x * texture(gDiffuseMap, pIn.pTexUV)) * gMat.diffuse;

	ps_spec = gMat.specular;
	ps_spec.w /= 255.0f;


	ps_diff.w = 1.0; // 1: lit, 0: shadow
	vec4 shadowTrans = pIn.pShadowTransform;

	shadowTrans.xyz /= shadowTrans.w;

	float epsilon = 0.003;
	float shadow = texture2D(gShadowMap, shadowTrans.xy).r;
	shadow += epsilon;
	if (shadow  < shadowTrans.z) {
		ps_diff.w = 0.0;
	}
}