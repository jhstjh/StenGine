#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <imgui.h>
#include <fstream>
#include "Scene/GameObjectManager.h"
#include "Scene/SceneFileManager.h"

using namespace rapidjson;

#if BUILD_DEBUG
#define DEFAULT_SCENE L"Scene/TestScene.sgs"
#else
#define DEFAULT_SCENE L"Scene/TestSceneFull.sgs"
#endif

namespace StenGine
{

class SceneFileManagerImpl : public SceneFileManager
{
public:
	void LoadScene(std::wstring path) final
	{
		Document sceneDoc;
		std::ifstream ifs(mLoadedScene);
		IStreamWrapper isw(ifs);

		auto ret = sceneDoc.ParseStream(isw).HasParseError();

		assert(!ret);
		assert(sceneDoc.IsObject());

		const Value& gameObjects = sceneDoc["GameObjects"];
		assert(gameObjects.IsArray());

		for (Value::ConstValueIterator itr = gameObjects.Begin(); itr != gameObjects.End(); ++itr)
		{
			const char* objectType = itr->FindMember("ObjectType")->value.GetString();
			const char* name = itr->FindMember("Name")->value.GetString();
			UUID uuid, parentUuid;
			UuidFromStringA((RPC_CSTR)(itr->FindMember("UUID")->value.GetString()), &uuid);
			
			auto transform = itr->FindMember("Transform");
			UuidFromStringA((RPC_CSTR)(transform->value.FindMember("Parent")->value.GetString()), &parentUuid);
			auto translation = transform->value.FindMember("Translation");
			float transX = translation->value.FindMember("x")->value.GetFloat();
			float transY = translation->value.FindMember("y")->value.GetFloat();
			float transZ = translation->value.FindMember("z")->value.GetFloat();
			auto rotation = transform->value.FindMember("Rotation");
			float rotX = rotation->value.FindMember("x")->value.GetFloat();
			float rotY = rotation->value.FindMember("y")->value.GetFloat();
			float rotZ = rotation->value.FindMember("z")->value.GetFloat();
			auto scale = transform->value.FindMember("Scale");
			float scaleX = scale->value.FindMember("x")->value.GetFloat();
			float scaleY = scale->value.FindMember("y")->value.GetFloat();
			float scaleZ = scale->value.FindMember("z")->value.GetFloat();

			GameObjectManager::Instance()->Instantiate(objectType, uuid, name, parentUuid, transX, transY, transZ, rotX, rotY, rotZ, scaleX, scaleY, scaleZ);
		}

		GameObjectManager::Instance()->BuildSceneHierarchy();

		ifs.close();
	}

	void Save() final
	{

	}
	
	void DrawMenu() final
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::MenuItem("Load");
				ImGui::MenuItem("Save");
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

private:
	std::wstring mLoadedScene{ DEFAULT_SCENE };
};

DEFINE_ABSTRACT_SINGLETON_CLASS(SceneFileManager, SceneFileManagerImpl);

}