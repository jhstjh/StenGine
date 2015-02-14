#define TERRAIN
#include "StandardConstant.fx"


float CalcTessFactor(float3 p)
{
	float d = distance(p, gEyePosW);

	// max norm in xz plane (useful to see detail levels from a bird's eye).
	//float d = max( abs(p.x-gEyePosW.x), abs(p.z-gEyePosW.z) );

	float s = saturate((d - gMinDist) / (gMaxDist - gMinDist));

	return pow(2, (lerp(gMaxTess, gMinTess, s)));
}

TerrainPatchTess ConstantHS(InputPatch<TerrainVertexOut, 4> patch,
							uint patchID : SV_PrimitiveID)
{
	TerrainPatchTess pt;

	// frustum culling?..

	if (false) {

	}
	else {
		float3 e0 = 0.5f*(patch[0].PosW + patch[2].PosW);
		float3 e1 = 0.5f*(patch[0].PosW + patch[1].PosW);
		float3 e2 = 0.5f*(patch[1].PosW + patch[3].PosW);
		float3 e3 = 0.5f*(patch[2].PosW + patch[3].PosW);
		float3 c = 0.25f*(patch[0].PosW + patch[1].PosW +
						  patch[2].PosW + patch[3].PosW);

		pt.EdgeTess[0] = CalcTessFactor(e0);
		pt.EdgeTess[1] = CalcTessFactor(e1);
		pt.EdgeTess[2] = CalcTessFactor(e2);
		pt.EdgeTess[3] = CalcTessFactor(e3);

		pt.InsideTess[0] = CalcTessFactor(c);
		pt.InsideTess[1] = pt.InsideTess[0];

		return pt;
	}

}

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
TerrainHullOut main(InputPatch<TerrainVertexOut, 4> p,
					uint i : SV_OutputControlPointID,
					uint patchId : SV_PrimitiveID)
{ 
	TerrainHullOut hOut;

	hOut.PosW	= p[i].PosW;
	hOut.TexUV	= p[i].TexUV;

	return hOut;
}