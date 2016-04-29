#version 450
#extension GL_ARB_bindless_texture : require

layout(std140) uniform imGuiCB
{
	mat4	  ProjMtx;
	sampler2D Texture;
};

in VertexOut
{
	vec2 Frag_UV;
	vec4 Frag_Color;
} pIn;

out vec4 ps_color;

void main()
{
	ps_color = pIn.Frag_Color * texture(Texture, pIn.Frag_UV.st);
}