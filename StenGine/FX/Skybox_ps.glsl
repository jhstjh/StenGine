#version 450
#extension GL_ARB_bindless_texture : require

in vec3 pPosL;

layout(std140) uniform ubPerObj{
	mat4 gWorldViewProj;
};

layout(std140) uniform ubTextures{
	samplerCube gCubeMap;
};

out vec4 svColor;

void main() {
	svColor = texture(gCubeMap, pPosL);
}
