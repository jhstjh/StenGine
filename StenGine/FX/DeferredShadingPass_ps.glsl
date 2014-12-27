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

out vec4 ps_color;

uniform sampler2D gDiffuseGMap;


layout(std140) uniform ubPerFrame {
	vec4 gEyePosW;
	DirectionalLight gDirLight;
};

void main() {
	ps_color = texture(gDiffuseGMap, vec2(pTexUV.x, 1 - pTexUV.y));
}