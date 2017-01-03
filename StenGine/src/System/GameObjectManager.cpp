#include "Scene/GameObjectManager.h"
#include "System/API/PlatformAPIDefs.h"
#include "Mesh/MeshRenderer.h"
#include "Mesh/SkinnedMesh.h"
#include "Resource/ResourceManager.h"
#include "Mesh/Terrain.h"
#include "Math/MathHelper.h"
#include "Graphics/Animation/Animation.h"
#include "imgui.h"
#include "Engine/EventSystem.h"

#define MIN_SCENE 1

namespace StenGine
{

DEFINE_SINGLETON_CLASS(GameObjectManager)

GameObjectManager::GameObjectManager()
{
	EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::UPDATE, [this]() {Update(); });
}

GameObjectManager::~GameObjectManager()
{
	for (auto &gameObject : mGameObjects)
	{
		delete gameObject;
	}
}

void GameObjectManager::LoadScene()
{

}

void GameObjectManager::Update()
{
	for (auto &gameObject : mGameObjects)
	{
		gameObject->Update();
	}
}

void GameObjectManager::DrawMenu()
{
	ImGui::Begin("Scene");

	static int32_t currentItem = -1;
	std::vector<const char*> names;
	for (size_t i = 0; i < mGameObjects.size(); ++i)
	{
		names.push_back(mGameObjects[i]->m_name.c_str());
	}

	ImGui::ListBox("", &currentItem, names.data(), (int32_t)names.size(), 10);
	ImGui::End();

	if (currentItem != -1)
	{
		ImGui::Begin("Inspector");
		mGameObjects[currentItem]->DrawMenu();
		ImGui::End();
	}
}

}