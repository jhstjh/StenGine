#include "Scene/GameObjectManager.h"
#include "System/API/PlatformAPIDefs.h"
#include "Mesh/MeshRenderer.h"
#include "Mesh/SkinnedMeshRenderer.h"
#include "Resource/ResourceManager.h"
#include "Mesh/Terrain/Terrain.h"
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
	EventSystem::Instance()->RegisterEventHandler(EventSystem::EventType::UPDATE_TRANSFORM, [this]() {UpdateTransform(); });
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

void GameObjectManager::UpdateTransform()
{
	mRoot.UpdateWorldTransform(Mat4::Identity(), false);
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
	static Transform* selected = nullptr;
	if (ImGui::Begin("Scene"), ImGuiTreeNodeFlags_DefaultOpen)
	{
		mRoot.DrawMenuNodes(selected, true);
		ImGui::End();
	}

	if (selected && selected != &mRoot)
	{
		if (ImGui::Begin("Inspector"))
		{
			selected->mParent->DrawMenu();
			ImGui::End();
		}
	}
}

}