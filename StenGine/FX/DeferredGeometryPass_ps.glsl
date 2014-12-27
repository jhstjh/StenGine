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
in vec3 pPosW;
in vec3 pTangV;

//out vec4 ps_color;
layout(location = 0) out vec4 ps_norm;
layout(location = 1) out vec4 ps_diff;
layout(location = 2) out vec4 ps_spec;

uniform sampler2D gDiffuseMap;
uniform sampler2D gNormalMap;


layout(std140) uniform ubPerObj {
	mat4 gWorldViewProj;
	mat4 gWorld;
	mat4 gWorldView;
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

	vec4 diffColor = vec4(0, 0, 0, 0);
	vec4 specColor = vec4(0, 0, 0, 0);

	ps_norm = vec4(normal, 1.0);
	ps_diff = texture(gDiffuseMap, pTexUV) * gMat.diffuse;
	ps_spec = gMat.specular;
	ps_spec.w /= 255.0f;
}