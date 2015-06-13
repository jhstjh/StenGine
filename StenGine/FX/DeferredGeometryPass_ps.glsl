#version 420

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

uniform sampler2D gDiffuseMap;
uniform sampler2D gNormalMap;
uniform sampler2D gShadowMap;
uniform samplerCube gCubeMap;

layout(std140, binding = 0) uniform ubPerObj{
	mat4 gWorldViewProj;
	mat4 gWorld;
	mat4 gWorldView;
	mat4 gShadowTransform;
	Material gMat;
	vec4 DiffX_NormY_ShadZ;
};

layout(std140, binding = 1) uniform ubPerFrame{
	vec4 gEyePosW;
	DirectionalLight gDirLight;
};

void main() {
	vec3 normal = normalize(pIn.pNormalV);

	vec3 normalMapNormal = texture(gNormalMap, pIn.pTexUV).xyz;
	normalMapNormal = 2.0f * normalMapNormal - 1.0;

	vec3 N = normalize(normal);
	vec3 T = normalize(pIn.pTangV - dot(pIn.pTangV, N)*N);
	vec3 B = cross(N, T);

	mat3 TBN = mat3(T, B, N);

	normal = normalize(TBN * normalMapNormal);//normalize(pin.NormalV).xy;

	ps_norm = (vec4(normal, 1.0) + 1) / 2.0f;
	ps_norm.w = 1.0f;

	vec3 eyeRay = normalize(pIn.pPosW - gEyePosW.xyz);
	vec3 refRay = reflect(eyeRay, pIn.pNormalW);

	if (DiffX_NormY_ShadZ.x > 0.5)
		ps_diff = texture(gDiffuseMap, pIn.pTexUV) * gMat.diffuse;
	else
		ps_diff = textureLod(gCubeMap, refRay, 7);

	ps_spec = gMat.specular;
	ps_spec.w /= 255.0f;


	ps_diff.w = 1.0; // 1: lit, 0: shadow
	vec4 shadowTrans = pIn.pShadowTransform;

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