#include "GameObject/GameTerrain.h"
#include "Mesh/MeshRenderer.h"
#include "Resource/ResourceManager.h"
#include "Mesh/Terrain/Terrain.h"

namespace SGGame
{

GameTerrain::GameTerrain()
{
	Terrain::InitInfo tii;
	tii.HeightMapFilename = L"Terrain/terrain.raw";
	tii.LayerMapFilenames.resize(5);
	tii.LayerMapFilenames[0] = L"Terrain/grass.dds";
	tii.LayerMapFilenames[1] = L"Terrain/darkdirt.dds";
	tii.LayerMapFilenames[2] = L"Terrain/stone.dds";
	tii.LayerMapFilenames[3] = L"Terrain/lightdirt.dds";
	tii.LayerMapFilenames[4] = L"Terrain/snow.dds";
	tii.BlendMapFilename = L"Terrain/blend.dds";
	tii.HeightScale = 50.0f;
	tii.HeightmapWidth = 2049;
	tii.HeightmapHeight = 2049;
	tii.CellSpacing = 0.5f;

	auto terrainComp = std::make_unique<Terrain>(tii);
	AddComponent(std::move(terrainComp));
}

}