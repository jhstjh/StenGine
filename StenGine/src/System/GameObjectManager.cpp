#include "Scene/GameObjectManager.h"
#include "System/API/PlatformAPIDefs.h"
#include "Mesh/MeshRenderer.h"
#include "Mesh/SkinnedMesh.h"
#include "Resource/ResourceManager.h"
#include "Mesh/Terrain.h"
#include "Math/MathHelper.h"
#include "imgui.h"

#define MIN_SCENE 1

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
	//GameObject* box0 = new GameObject("box0", 0.f, 1.2f, 0.f, 0.f, PI / 5);
	//Mesh* box0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"GenerateBox");
	//box0->AddComponent(box0Mesh);
	//m_gameObjects.push_back(box0);
	//
	//
	//GameObject* sphere = new GameObject("sphere", 0.f, 3.7f, -0.5f);
	//Mesh* sphereMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/earth.fbx");
	//sphere->AddComponent(sphereMesh);
	//m_gameObjects.push_back(sphere);


	GameObject* zombie = new GameObject("Zombie", 0.f, 3.7f, -0.5f);
	SkinnedMesh* zombieMesh = ResourceManager::Instance()->GetResource<SkinnedMesh>(L"Model/JointFBX.fbx");
	zombie->AddComponent(zombieMesh);
	m_gameObjects.push_back(zombie);


	

#if !MIN_SCENE || BUILD_RELEASE

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

	GameObject* house0 = new GameObject("house0", 0.f, -0.1f, 20.f, 0.f, PI/2);
	Mesh* house0Mesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/house.fbx");
	house0->AddComponent(house0Mesh);
	m_gameObjects.push_back(house0);

	GameObject* dragon = new GameObject("dragon", 3.f, 0.2f, 0.f);
	Mesh* dragonMesh = ResourceManager::Instance()->GetResource<Mesh>(L"Model/dragon.fbx");
	dragon->AddComponent(dragonMesh);
	m_gameObjects.push_back(dragon);


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

	GameObject* terrain = new GameObject("Terrain", 0, 0, -100);

	Terrain* terrainComp = new Terrain(tii);
	terrain->AddComponent(terrainComp);

	m_gameObjects.push_back(terrain);
#endif
}

void GameObjectManager::Update()
{
	for (auto &gameObject : m_gameObjects)
	{
		gameObject->Update();
	}
}

void GameObjectManager::DrawMenu()
{
	ImGui::Begin("Scene");

	static int32_t currentItem = -1;
	std::vector<const char*> names;
	for (size_t i = 0; i < m_gameObjects.size(); ++i)
	{
		names.push_back(m_gameObjects[i]->m_name.c_str());
	}

	ImGui::ListBox("", &currentItem, names.data(), (int32_t)names.size(), 10);
	ImGui::End();

	if (currentItem != -1)
	{
		ImGui::Begin("Inspector");
		m_gameObjects[currentItem]->DrawMenu();
		ImGui::End();
	}
}

}