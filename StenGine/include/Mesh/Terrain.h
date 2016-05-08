#pragma once
#include "Graphics/D3DIncludes.h"
#include "Graphics/Effect/Material.h"
#include "Scene/Component.h"
#include "Scene/Drawable.h"
#include "Graphics/Abstraction/GPUBuffer.h"
#include "Graphics/Abstraction/Texture.h"

namespace StenGine
{

class Terrain : public Component, public Drawable {
public:
	struct InitInfo {
		std::wstring HeightMapFilename;
		std::vector<std::wstring> LayerMapFilenames;
		std::wstring BlendMapFilename;
		float HeightScale;
		UINT HeightmapWidth;
		UINT HeightmapHeight;
		float CellSpacing;
	};

	Terrain(struct Terrain::InitInfo &info);
	~Terrain();

	virtual void GatherDrawCall() override;
	virtual void GatherShadowDrawCall() override;

	virtual void DrawMenu() override;

private:
	static const int CellsPerPatch = 64;

	void LoadHeightmap();
	void Smooth();
	bool InBounds(int i, int j);
	float Average(int i, int j);
	void BuildHeightMapSRV();

	float GetWidth() const;
	float GetDepth() const;
	float GetHeight(float x, float z) const;

	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT i, UINT j);

	void BuildQuadPatchVB();
	void BuildQuadPatchIB();

	InitInfo m_initInfo;

	GPUBuffer* m_quadPatchVB;
	GPUBuffer* m_quadPatchIB;

	Texture* m_layerMapArrayTex;
	Texture* m_blendMapTex;
	Texture* m_heightMapTex;

	UINT m_numPatchVertices;
	UINT m_numPatchQuadFaces;

	UINT m_numPatchVertRows;
	UINT m_numPatchVertCols;

	Material m_material;

	std::vector<XMFLOAT2> m_patchBoundsY;
	std::vector<float> m_heightMap;
};

}