#include "Scene/GameObjectManager.h"
#include "System/API/PlatformAPIDefs.h"
#include "Mesh/MeshRenderer.h"
#include "Resource/ResourceManager.h"
#include "Mesh/Terrain.h"

#define MIN_SCENE 0

namespace StenGine
{

DEFINE_SINGLETON_CLASS(GameObjectManager)

GameObjectManager::~GameObjectManager()
{
	for (auto &gameObject : m_gameObjects)
	{
		delete gameObject;
	}
}

void GameObjectManager::LoadScene()
{
	GameObject* box0 = new GameObject("box0", 0.f, 1.2f, 0.f);
	Mesh* box0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"GenerateBox");
	box0->AddComponent(box0Mesh);
	box0->RotateAroundY(3.14159f / 5);
	m_gameObjects.push_back(box0);


	GameObject* sphere = new GameObject("sphere", 0.f, 3.7f, -0.5f);
	Mesh* sphereMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/earth.fbx");
	sphere->AddComponent(sphereMesh);
	m_gameObjects.push_back(sphere);
#if !MIN_SCENE

	GameObject* plane0 = new GameObject("plane0", 4.f, 0.2f, 0.f);
	Mesh* plane0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/plane.fbx");
	plane0->AddComponent(plane0Mesh);
	m_gameObjects.push_back(plane0);

	GameObject* plane1 = new GameObject("plane1", -4.f, 0.2f, 0.f);
	Mesh* plane1Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/plane.fbx");
	plane1->AddComponent(plane1Mesh);
	m_gameObjects.push_back(plane1);

	GameObject* plants0 = new GameObject("plants0", -4.f, 0.2f, 0.f);
	Mesh* plants0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/plants.fbx");
	plants0->AddComponent(plants0Mesh);
	m_gameObjects.push_back(plants0);

	GameObject* house0 = new GameObject("plants0", 0.f, -0.1f, 20.f);
	Mesh* house0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/house.fbx");
	house0->AddComponent(house0Mesh);
	house0->RotateAroundY(3.1415926f / 2);
	m_gameObjects.push_back(house0);

	GameObject* dragon = new GameObject("dragon", 3, 0.2, 0);
	Mesh* dragonMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/dragon.fbx");
	dragon->AddComponent(dragonMesh);
	m_gameObjects.push_back(dragon);


//	Terrain::InitInfo tii;
//	tii.HeightMapFilename = L"Terrain/terrain.raw";
//	tii.LayerMapFilenames.resize(5);
//	tii.LayerMapFilenames[0] = L"Terrain/grass.dds";
//	tii.LayerMapFilenames[1] = L"Terrain/darkdirt.dds";
//	tii.LayerMapFilenames[2] = L"Terrain/stone.dds";
//	tii.LayerMapFilenames[3] = L"Terrain/lightdirt.dds";
//	tii.LayerMapFilenames[4] = L"Terrain/snow.dds";
//	tii.BlendMapFilename = L"Terrain/blend.dds";
//	tii.HeightScale = 50.0f;
//	tii.HeightmapWidth = 2049;
//	tii.HeightmapHeight = 2049;
//	tii.CellSpacing = 0.5f;
//
//	GameObject* terrain = new GameObject("Terrain", 0, 0, -100);
//
//	Terrain* terrainComp = new Terrain(tii);
//	terrain->AddComponent(terrainComp);
//
//	m_gameObjects.push_back(terrain);
#endif
}

void GameObjectManager::Update()
{
	for (auto &gameObject : m_gameObjects)
	{
		gameObject->Update();
	}
}

}