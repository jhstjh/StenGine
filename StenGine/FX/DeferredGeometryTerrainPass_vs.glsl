#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_EXT_texture_array : enable

// input layout
layout(location = 0) in vec3 PosL;
layout(location = 1) in vec2 TexUV;
layout(location = 2) in vec2 BoundsY;

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

layout(std140) uniform ubPerObj {
	mat4 gWorldViewProj;
	mat4 gWorldViewInvTranspose;
	mat4 gWorldInvTranspose;
	mat4 gWorldView;
	mat4 gWorld;
	mat4 gViewProj;
	mat4 gShadowTransform;
	mat4 gView;

	Material gMat;
	vec4 DiffX_NormY_ShadZ;
	
	sampler2D		gShadowMap;
	samplerCube		gCubeMap;
	sampler2D		gHeightMap;
	sampler2DArray	gLayerMapArray;
	sampler2D		gBlendMap;
};

out TerrainVertexOut{
	vec3 PosW;
	vec2 TexUV;
	vec2 BoundsY;
} vOut;

void main() {
	vOut.PosW = PosL;
	//vOut.PosW.y = textureLod(gHeightMap, TexUV, 0).r;

	vOut.PosW = (gWorld * vec4(vOut.PosW, 1.0f)).xyz;

	vOut.TexUV = TexUV;
	vOut.BoundsY = BoundsY;
}
