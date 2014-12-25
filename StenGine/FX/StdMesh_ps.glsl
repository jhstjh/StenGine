#version 410

in vec2 pTexUV;
in vec3 pNormalW;
in vec3 pPosW;

out vec4 ps_color;

uniform sampler2D gDiffuseMap;

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