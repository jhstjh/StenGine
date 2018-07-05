#pragma once
#include "Graphics/Abstraction/GPUBuffer.h"
#include "Graphics/Abstraction/Texture.h"
#include "Graphics/D3DIncludes.h"
#include "Graphics/Effect/Material.h"
#include "Math/MathDefs.h"
#include "Scene/Component.h"
#include "Scene/Drawable.h"
#include "TerrainGrass.h"

namespace StenGine
{

class Terrain : public Component, public Drawable {
public:
	struct InitInfo {
		std::wstring HeightMapFilename;
		std::vector<std::wstring> LayerMapFilenames;
		std::wstring BlendMapFilename;
		float HeightScale;
		uint32_t HeightmapWidth;
		uint32_t HeightmapHeight;
		float CellSpacing;
	};

	Terrain(struct Terrain::InitInfo &info);
	virtual ~Terrain();

	void GatherDrawCall() override;
	void GatherShadowDrawCall() override;

	void DrawMenu() override;
	float GetHeight(float x, float z) const;	
	const Mat4 &GetWorldTransform();

private:
	static const int CellsPerPatch = 64;

	void LoadHeightmap();
	void Smooth();
	bool InBounds(int i, int j);
	float Average(int i, int j);
	void BuildHeightMapSRV();

	float GetWidth() const;
	float GetDepth() const;

	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(uint32_t i, uint32_t j);

	void BuildQuadPatchVB();
	void BuildQuadPatchIB();

	InitInfo m_initInfo;

	GPUBuffer m_quadPatchVB;
	GPUBuffer m_quadPatchIB;

	Texture m_layerMapArrayTex;
	Texture m_blendMapTex;
	Texture m_heightMapTex;

	uint32_t m_numPatchVertices;
	uint32_t m_numPatchQuadFaces;

	uint32_t m_numPatchVertRows;
	uint32_t m_numPatchVertCols;

	Material m_material;

	std::vector<Vec2Packed> m_patchBoundsY;
	std::vector<float> m_heightMap;

	std::vector<TerrainGrass> mGrasses;
};

}