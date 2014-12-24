#version 410

in vec2 pTexUV;
out vec4 ps_color;

uniform sampler2D gDiffuseMap;

void main() {
	vec4 color = texture(gDiffuseMap, pTexUV);//vec4(0.5, 0.5, 0.5, 1.0);
	ps_color = color;
}