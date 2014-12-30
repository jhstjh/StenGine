#version 410

struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct DirectionalLight {
	vec4 intensity;
	vec3 direction;
	float pad;
};

in vec2 pTexUV;
in vec3 pNormalV;
in vec3 pNormalW;
in vec3 pPosW;
in vec3 pTangV;
in vec4 pShadowTransform;

//out vec4 ps_color;
layout(location = 0) out vec4 ps_norm;
layout(location = 1) out vec4 ps_diff;
layout(location = 2) out vec4 ps_spec;

uniform sampler2D gDiffuseMap;
uniform sampler2D gNormalMap;
uniform sampler2D gShadowMap;
uniform samplerCube gCubeMap;

layout(std140) uniform ubPerObj {
	mat4 gWorldViewProj;
	mat4 gWorld;
	mat4 gWorldView;
	mat4 gShadowTransform;
	Material gMat;
	vec4 DiffX_NormY_ShadZ;
};

layout(std140) uniform ubPerFrame {
	vec4 gEyePosW;
	DirectionalLight gDirLight;
};

void main() {
	vec3 normal = normalize(pNormalV);

	vec3 normalMapNormal = texture(gNormalMap, pTexUV).xyz;
	normalMapNormal = 2.0f * normalMapNormal - 1.0;

	vec3 N = normalize(normal);
	vec3 T = normalize(pTangV - dot(pTangV, N)*N);
	vec3 B = cross(N, T);

	mat3 TBN = mat3(T, B, N);

	normal = normalize(TBN * normalMapNormal);//normalize(pin.NormalV).xy;

	ps_norm = vec4(normal, 1.0);
	ps_diff = texture(gDiffuseMap, pTexUV) * gMat.diffuse;
	ps_spec = gMat.specular;
	ps_spec.w /= 255.0f;


	ps_diff.w = 1.0; // 1: lit, 0: shadow
	vec4 shadowTrans = pShadowTransform;

	shadowTrans.xyz /= shadowTrans.w;

	//ps_diff = vec4(texture2D(gShadowMap, shadowTrans.xy).r, texture2D(gShadowMap, shadowTrans.xy).r, texture2D(gShadowMap, shadowTrans.xy).r, 1);
	//ps_diff = vec4(shadowTrans.z, shadowTrans.z, shadowTrans.z, 1);
	//return;

	float epsilon = 0.003;
	float shadow = texture2D(gShadowMap, shadowTrans.xy).r;
	if (shadow + epsilon < shadowTrans.z) {
		ps_diff.w = 0.0;
	}

	//vec3 viewRay = normalize(pPosW - gEyePosW.xyz);
	//vec3 refRay = reflect(viewRay, pNormalW);
	//ps_spec = texture(gCubeMap, refRay);

}