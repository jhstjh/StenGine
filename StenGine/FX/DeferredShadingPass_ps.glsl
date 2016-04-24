#version 450
#extension GL_ARB_bindless_texture : require

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

layout(location = 0) out vec4 ps_color;
layout(location = 1) out vec4 ps_ssao;

uniform float  gOcclusionRadius = 0.16f;
uniform float  gOcclusionFadeStart = 0.2f;
uniform float  gOcclusionFadeEnd = 8.0f;
uniform float  gSurfaceEpsilon = 0.005f;

uniform vec4 OffsetVect[] = {
	{ +1.0f, +1.0f, +1.0f, 0.0f },
	{ -1.0f, -1.0f, -1.0f, 0.0f },
	{ -1.0f, +1.0f, +1.0f, 0.0f },
	{ +1.0f, -1.0f, -1.0f, 0.0f },
	{ +1.0f, +1.0f, -1.0f, 0.0f },
	{ -1.0f, -1.0f, +1.0f, 0.0f },
	{ -1.0f, +1.0f, -1.0f, 0.0f },
	{ +1.0f, -1.0f, +1.0f, 0.0f },
	{ -1.0f, 0.0f, 0.0f, 0.0f },
	{ +1.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, -1.0f, 0.0f, 0.0f },
	{ 0.0f, +1.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, -1.0f, 0.0f },
	{ 0.0f, 0.0f, +1.0f, 0.0f },
};

uniform int gBlurRadius = 10;


layout(std140) uniform ubPerFrame {
	DirectionalLight gDirLight;
	vec4 gEyePosV;
	mat4 gProjInv;
	mat4 gProj;
	sampler2D gNormalGMap;
	sampler2D gDiffuseGMap;
	sampler2D gSpecularGMap;
	sampler2D gDepthGMap;
	sampler2D gRandVectMap;
};

float OcclusionFunction(float distZ)
{
	//
	// If depth(q) is "behind" depth(p), then q cannot occlude p.  Moreover, if 
	// depth(q) and depth(p) are sufficiently close, then we also assume q cannot
	// occlude p because q needs to be in front of p by Epsilon to occlude p.
	//
	// We use the following function to determine the occlusion.  
	// 
	//
	//						 1.0     -------------\
					//               |           |  \
					//               |           |    \
					//               |           |      \ 
					//               |           |        \
					//               |           |          \
					//               |           |            \
					//  ------|------|-----------|-------------|---------|--> zv
					//        0     Eps          z0            z1        
	//

	float occlusion = 0.0f;
	if (distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;

		// Linearly decrease occlusion from 1 to 0 as distZ goes 
		// from gOcclusionFadeStart to gOcclusionFadeEnd.	
		occlusion = clamp((gOcclusionFadeEnd - distZ) / fadeLength, 0, 1);
	}

	return occlusion;
}

void main() {

	vec2 realUV = pTexUV;

	vec3 normalV = texture(gNormalGMap, realUV).xyz * 2 - 1;
	vec3 specularFactor = texture(gSpecularGMap, realUV).xyz;
	vec3 diffuseFactor = texture(gDiffuseGMap, realUV).xyz;

	float z = texture(gDepthGMap, realUV).x;

	float x = pTexUV.x * 2 - 1;
	float y = (pTexUV.y) * 2 - 1; // NOT 1 - y !!!
	vec4 vProjectedPos = vec4(x, y, z, 1.0f);
	vec4 vPositionVS = gProjInv * vProjectedPos;
	vPositionVS.xyz /= vPositionVS.w;

	if (1000 - vPositionVS.z - 1 < 0) discard;

	vec4 diffColor = vec4(0, 0, 0, 0);
	vec4 specColor = vec4(0, 0, 0, 0);

	float diffuseK = dot(-gDirLight.direction, normalV);
	float shadowLit = texture(gDiffuseGMap, realUV).w;
//
//	if (diffuseK > 0) {
//		diffColor += diffuseK * gDirLight.intensity;
//		vec3 refLight = reflect(gDirLight.direction, normalV);
//		vec3 viewRay = vec3(0, 0, 0) - vPositionVS.xyz;
//		viewRay = normalize(viewRay);
//		specColor += texture(gSpecularGMap, realUV) * pow(max(dot(refLight, viewRay), 0), texture(gSpecularGMap, realUV).w * 255);
//	}
//
//	ps_color = (vec4(0.2, 0.2, 0.2, 0) + diffColor * shadowLit) * texture(gDiffuseGMap, realUV) + specColor * shadowLit /*+ float4(1, 1, 1, 1) * vPositionVS.z / 20*/;
//	//ps_color = texture(gDiffuseGMap, realUV);

	float roughnessFactor = 0.1;

	vec3 viewRay = normalize(vec3(0, 0, 0) - vPositionVS.xyz);
	vec3 light = -gDirLight.direction;

	vec3 halfVec = normalize(light + viewRay);
	float NdotL = clamp(dot(normalV, light), 0.0, 1.0);
	float NdotH = clamp(dot(normalV, halfVec), 0.0, 1.0);
	float NdotV = clamp(dot(normalV, viewRay), 0.0, 1.0);
	float VdotH = clamp(dot(viewRay, halfVec), 0.0, 1.0);
	float LdotH = clamp(dot(light, halfVec), 0.0, 1.0);
	float r_sq = roughnessFactor * roughnessFactor;


	float geoNumerator = 2.0f * NdotH;
	float geoDenominator = VdotH;

	float geoB = (geoNumerator * NdotV) / VdotH;
	float geoC = (geoNumerator * NdotL) / LdotH;
	float geo = min(1.0, min(geoB, geoC));


	float roughness;

//	float roughness_a = 1.0f / (4.0f * r_sq * pow(NdotH, 4));
//	float roughness_b = NdotH * NdotH - 1.0;
//	float roughness_c = r_sq * NdotH * NdotH;

//	roughness = roughness_a * exp(roughness_b / roughness_c);
	float c = 1.0f;
	float alpha = acos(dot(normalV, halfVec));
	roughness = c * exp(-(alpha / r_sq));


	roughness = 1.0 / (3.1415926 * r_sq * pow(NdotH, 4)) * exp((NdotH * NdotH - 1) / (r_sq * NdotH * NdotH));


	
	float ref_at_norm_incidence = 0.8;
	float fresnel = pow(1.0 - VdotH, 5.0f);
	fresnel *= (1.0f - ref_at_norm_incidence);
	fresnel += ref_at_norm_incidence;

	float rsnum1d = fresnel * geo * roughness;
	vec3 Rs_numerator = vec3(rsnum1d, rsnum1d, rsnum1d);
	float Rs_denominator = NdotV * NdotL * 3.1415926;
	vec3 Rs = Rs_numerator / Rs_denominator;

	vec3 cDiffuse = diffuseFactor * ((max(0, diffuseK) * gDirLight.intensity.xyz) * shadowLit);


	vec3 final = max(0.0f, NdotL) * (clamp(specularFactor * Rs * shadowLit, 0.0, 1.0) + cDiffuse) + vec3(0.2, 0.2, 0.2) * diffuseFactor;

	//vec3 r = vec3(0.2, 0.2, 0.2) + NdotL * (0.8 + (1 - 0.8) * Rs);

	//ps_color = vec4(r * diffuseFactor, 1);
	ps_color = vec4(final, 1.0);

	gl_FragDepth = z; // weite GBUFFER depth to current depth buffer


	/*******************SSAO**********************/

	vec3 randVect = normalize(textureLod(gRandVectMap, 16 * pTexUV, 0)).xyz;

	float occlusionSum = 0.0f;
	vec4 occlusionColor = vec4(0, 0, 0, 0);
	for (int i = 0; i < 14; ++i)
	{
		vec3 offset = reflect(normalize(OffsetVect[i].xyz), randVect);

		float flip = 1;//sign(dot(offset, normal));

		vec3 qV = vPositionVS.xyz + flip * gOcclusionRadius * offset;

		vec4 projQ = gProj * vec4(qV, 1.0f);
		projQ /= projQ.w;

		float rz = textureLod(gDepthGMap, vec2(0.5 * projQ.x + 0.5, (0.5 + 0.5 * projQ.y)), 0).r;
		vec4 rProjectedPos = vec4(projQ.x, projQ.y, rz, 1.0f);
		vec4 rPositionVS = gProjInv * rProjectedPos;
		rPositionVS /= rPositionVS.w;

		vec3 diff = rPositionVS.xyz - vPositionVS.xyz;
		const vec3 v = normalize(diff);
		const float d = length(diff)*0.05;
		float occlusion = max(0.0, dot(normalV, v) - gSurfaceEpsilon) * (1.0 / (1.0 + d)) * 0.8;
		occlusionSum += occlusion;
	}

	occlusionSum /= 14.0f;

	float access = 1.0f - occlusionSum;
	ps_ssao = vec4(clamp(pow(access, 4.0f), 0.0f, 1.0f));
}