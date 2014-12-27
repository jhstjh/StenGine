#version 410

struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct DirectionalLight {
	vec4 intensity;
	vec3 direction;
	float pad;
};

in vec2 pTexUV;

out vec4 ps_color;


uniform sampler2D gNormalGMap;
uniform sampler2D gDiffuseGMap;
uniform sampler2D gSpecularGMap;
uniform sampler2D gDepthGMap;


layout(std140) uniform ubPerFrame {
	DirectionalLight gDirLight;
	vec4 gEyePosV;
	mat4 gProjInv;
	mat4 gProj;
};

void main() {
	vec2 realUV = pTexUV;

	vec3 normalV = texture(gNormalGMap, realUV).xyz;
	float z = texture(gDepthGMap, realUV).x;

	float x = pTexUV.x * 2 - 1;
	float y = (1 - pTexUV.y) * 2 - 1;
	vec4 vProjectedPos = vec4(x, y, z, 1.0f);
	vec4 vPositionVS = gProjInv * vProjectedPos;
	vPositionVS.xyz /= vPositionVS.w;

	if (1000 - vPositionVS.z - 1 < 0) discard;

	vec4 diffColor = vec4(0, 0, 0, 0);
	vec4 specColor = vec4(0, 0, 0, 0);

	float diffuseK = dot(-gDirLight.direction, normalV);

	if (diffuseK > 0) {
		diffColor += diffuseK * gDirLight.intensity;
		vec3 refLight = reflect(gDirLight.direction, normalV);
		vec3 viewRay = vec3(0, 0, 0) - vPositionVS.xyz;
		viewRay = normalize(viewRay);
		specColor += texture(gSpecularGMap, realUV) * pow(max(dot(refLight, viewRay), 0), texture(gSpecularGMap, realUV).w * 255);
	}

	ps_color = (vec4(0.2, 0.2, 0.2, 0) + diffColor) * texture(gDiffuseGMap, realUV) + specColor /*+ float4(1, 1, 1, 1) * vPositionVS.z / 20*/;
}