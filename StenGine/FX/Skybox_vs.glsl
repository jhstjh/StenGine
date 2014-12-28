#version 410

in int gl_VertexID;

out vec3 pPosL;

layout(std140) uniform ubPerObj{
	mat4 gWorldViewProj;
};

uniform vec3 vertexArray[24] = {
	vec3(-1.0f, -1.0f, -1.0f),
	vec3(-1.0f, +1.0f, -1.0f),
	vec3(+1.0f, +1.0f, -1.0f),
	vec3(+1.0f, -1.0f, -1.0f),
	vec3(-1.0f, -1.0f, +1.0f),
	vec3(+1.0f, -1.0f, +1.0f),
	vec3(+1.0f, +1.0f, +1.0f),
	vec3(-1.0f, +1.0f, +1.0f),
	vec3(-1.0f, +1.0f, -1.0f),
	vec3(-1.0f, +1.0f, +1.0f),
	vec3(+1.0f, +1.0f, +1.0f),
	vec3(+1.0f, +1.0f, -1.0f),
	vec3(-1.0f, -1.0f, -1.0f),
	vec3(+1.0f, -1.0f, -1.0f),
	vec3(+1.0f, -1.0f, +1.0f),
	vec3(-1.0f, -1.0f, +1.0f),
	vec3(-1.0f, -1.0f, +1.0f),
	vec3(-1.0f, +1.0f, +1.0f),
	vec3(-1.0f, +1.0f, -1.0f),
	vec3(-1.0f, -1.0f, -1.0f),
	vec3(+1.0f, -1.0f, -1.0f),
	vec3(+1.0f, +1.0f, -1.0f),
	vec3(+1.0f, +1.0f, +1.0f),
	vec3(+1.0f, -1.0f, +1.0f),
};

uniform uint indexArray[36] = {
	1, 0, 2,
	2, 0, 3,

	5, 4, 6,
	6, 4, 7,

	9, 8, 10,
	10, 8, 11,

	13, 12, 14,
	14, 12, 15,

	17, 16, 18,
	18, 16, 19,

	21, 20, 22,
	22, 20, 23
};

void main() {
	pPosL = vertexArray[indexArray[gl_VertexID]];
	gl_Position = (gWorldViewProj * vec4(pPosL, 1.0)).xyww;
	gl_Position.z -= 0.001;
}