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
in vec3 pNormalW;
in vec3 pPosW;
in vec3 pTangW;

//out vec4 ps_color;
layout(location = 0) out vec4 ps_norm;
layout(location = 1) out vec4 ps_diff;
layout(location = 2) out vec4 ps_spec;

uniform sampler2D gDiffuseMap;
uniform sampler2D gNormalMap;


layout(std140) uniform ubPerObj {
	mat4 gWorldViewProj;
	mat4 gWorld;
	Material gMat;
	vec4 DiffX_NormY_ShadZ;
};

layout(std140) uniform ubPerFrame {
	vec4 gEyePosW;
	DirectionalLight gDirLight;
};

void main() {
	vec3 normal = normalize(pNormalW);

	vec3 normalMapNormal = texture(gNormalMap, pTexUV).xyz;
	normalMapNormal = 2.0f * normalMapNormal - 1.0;

	vec3 N = normalize(normal);
	vec3 T = normalize(pTangW - dot(pTangW, N)*N);
	vec3 B = cross(N, T);

	mat3 TBN = mat3(T, B, N);

	normal = normalize(TBN * normalMapNormal);//normalize(pin.NormalV).xy;

	vec4 diffColor = vec4(0, 0, 0, 0);
	vec4 specColor = vec4(0, 0, 0, 0);

	float diffuseK = dot(-gDirLight.direction, normal);

	if (diffuseK > 0) {
		diffColor += diffuseK * gMat.diffuse * gDirLight.intensity;
		vec3 refLight = reflect(gDirLight.direction, normal);
		vec3 viewRay = gEyePosW.xyz - pPosW;
		viewRay = normalize(viewRay);
		specColor += gMat.specular * pow(max(dot(refLight, viewRay), 0), gMat.specular.w);
	}

	//ps_color = ((gMat.ambient + diffColor) * texture(gDiffuseMap, pTexUV) + specColor);
	ps_norm = vec4(normalMapNormal, 1.0) * 0.5 + 0.5;
	ps_diff = texture(gDiffuseMap, pTexUV);
	ps_spec = gMat.specular;
}