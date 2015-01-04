#version 410

layout(location = 0) in vec3 PosW;

in int gl_VertexID;
out vec4 pColor;

layout(std140) uniform ubPerObj {
	mat4 gViewProj;
};

void main() {

	//vOut.Color = vIn.Color;
	gl_Position = gViewProj * vec4(PosW, 1.0f);

	if (gl_VertexID < 2) pColor = vec4(1, 0, 0, 1);
	else if (gl_VertexID < 4) pColor = vec4(0, 1, 0, 1);
	else if (gl_VertexID < 6) pColor = vec4(0, 0, 1, 1);
	else pColor = vec4(0.5, 0.5, 0.5, 1);
}