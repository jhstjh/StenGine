#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <imgui.h>
#include <fstream>
#include "Scene/CameraManager.h"
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
			auto enabled = itr->FindMember("Enabled");

			auto components = itr->FindMember("Components");

			// Special component Transform
			auto transform = components->value.FindMember("Transform");
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

			GameObject* gameObject = GameObjectManager::Instance()->Instantiate(objectType, uuid, name, parentUuid, transX, transY, transZ, rotX, rotY, rotZ, scaleX, scaleY, scaleZ);
			gameObject->SetEnabled(enabled->value.GetBool());
			// other components
			{
				for (auto compIter = components->value.MemberBegin(); compIter != components->value.MemberEnd(); ++compIter)
				{
					if (compIter->name == "Transform")
					{
						continue;
					}
					else if (compIter->name == "Camera")
					{
						float fov = compIter->value.FindMember("FOV")->value.GetFloat();
						float np = compIter->value.FindMember("NearPlane")->value.GetFloat();
						float fp = compIter->value.FindMember("FarPlane")->value.GetFloat();

						auto camera = std::make_unique<Camera>(fov, np, fp);
						camera->SetEnabled(true);
						gameObject->AddComponent(std::move(camera));
					}
					// TODO move other hardcoded components to scene file and init here
				}

			}
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