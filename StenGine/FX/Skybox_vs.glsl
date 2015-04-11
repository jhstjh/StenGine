#version 410

layout(location = 0) in vec3 PosL;

in int gl_VertexID;

out vec3 pPosL;

layout(std140) uniform ubPerObj{
	mat4 gWorldViewProj;
};

void main() {
	pPosL = PosL;
	gl_Position = (gWorldViewProj * vec4(pPosL, 1.0)).xyww;
	gl_Position.z -= 0.001;
}