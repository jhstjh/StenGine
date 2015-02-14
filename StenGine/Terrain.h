#pragma once
#include "D3DIncludes.h"
#include "Component.h"
#include "Material.h"

class Terrain : public Component {
public:

	static Terrain* Instance() { return _instance; }

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

	void Draw();

private:
	static const int CellsPerPatch = 64;
	static Terrain* _instance;

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

	ID3D11Buffer* m_quadPatchVB;
	ID3D11Buffer* m_quadPatchIB;

	ID3D11ShaderResourceView* m_layerMapArraySRV;
	ID3D11ShaderResourceView* m_blendMapSRV;
	ID3D11ShaderResourceView* m_heightMapSRV;
	
	UINT m_numPatchVertices;
	UINT m_numPatchQuadFaces;

	UINT m_numPatchVertRows;
	UINT m_numPatchVertCols;

	Material m_material;

	std::vector<XMFLOAT2> m_patchBoundsY;
	std::vector<float> m_heightMap;
};