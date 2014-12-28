#version 410

in vec3 pPosL;

uniform samplerCube gCubeMap;
out vec4 svColor;

void main() {
	svColor = texture(gCubeMap, pPosL);
}
