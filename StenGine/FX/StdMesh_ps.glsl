#version 410

in vec2 pTexUV;
in vec3 pNormalW;
in vec3 pPosW;
in vec3 pTangW;

out vec4 ps_color;

uniform sampler2D gDiffuseMap;
uniform sampler2D gNormalMap;

// light info
uniform vec4 intensity;
uniform vec3 direction;

uniform vec3 gEyePosW;

// material
uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;

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

	float diffuseK = dot(-direction, normal);

	if (diffuseK > 0) {
		diffColor += diffuseK * diffuse * intensity;
		vec3 refLight = reflect(direction, normal);
		vec3 viewRay = gEyePosW - pPosW;
		viewRay = normalize(viewRay);
		specColor += specular * pow(max(dot(refLight, viewRay), 0), specular.w);
	}

	ps_color = ((ambient + diffColor) * texture(gDiffuseMap, pTexUV) + specColor);
}