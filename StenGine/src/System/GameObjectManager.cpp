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
	for (auto &entry : mGameObjects)
	{
		delete entry.second;
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
	for (auto &entry : mGameObjects)
	{
		entry.second->Update();
	}
}

void GameObjectManager::BuildSceneHierarchy()
{
	for (auto &entry : mGameObjects)
	{
		RPC_STATUS status;
		if (UuidIsNil(const_cast<UUID*>(&entry.second->m_parentUUID), &status))
		{
			mRoot.AddChild(entry.second->m_transform);
		}
		else
		{
			auto parentEntry = mGameObjects.find(entry.second->m_parentUUID);
			assert(parentEntry != mGameObjects.end());

			parentEntry->second->m_transform->AddChild(entry.second->m_transform);
		}
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