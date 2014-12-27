#version 410

in int gl_VertexID;
out vec2 pTexUV;

uniform vec4 vertexArray[6] = {
	vec4(-1.0, -1.0, 0.0, 1.0),
	vec4(-1.0, 1.0, 0.0, 1.0),
	vec4(1.0, 1.0, 0.0, 1.0),
	vec4(1.0, 1.0, 0.0, 1.0),
	vec4(1.0, -1.0, 0.0, 1.0),
	vec4(-1.0, -1.0, 0.0, 1.0),
};

uniform vec2 uvArray[6] = {
	vec2(0, 1),
	vec2(0, 0),
	vec2(1, 0),
	vec2(1, 0),
	vec2(1, 1),
	vec2(0, 1),
};

void main() {
	gl_Position = vertexArray[gl_VertexID];
	pTexUV = uvArray[gl_VertexID];
}
