#version 450
#extension GL_ARB_bindless_texture : require

layout(std140) uniform imGuiCB
{
	mat4	  ProjMtx;
	sampler2D Texture;
};

layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec4 Color;

out VertexOut
{
	vec2 Frag_UV;
	vec4 Frag_Color;
} vOut;

void main()
{
	vOut.Frag_UV = UV;
	vOut.Frag_Color = Color;
	gl_Position = ProjMtx * vec4(Position.xy,0,1);
}