#include <rapidjson/document.h>
#include <imgui.h>
#include "Scene/GameObjectManager.h"
#include "Scene/SceneFileManager.h"

const char* testScene =
"{\n"
"	\"GameObjects\" :[\n"
"		{\n"
"			\"ObjectType\" : \"Box\",\n"
"			\"UUID\" : \"E07B5012-320F-45B2-8EB0-6ADD00277038\",\n"
"			\"Name\" : \"box0\",\n"
"			\"Transform\" : {\n"
"				\"Parent\" : \"00000000-0000-0000-0000-000000000000\",\n"
"				\"Translation\" : {\n"
"					\"x\" : 0.0,\n"
"					\"y\" : 1.2,\n"
"					\"z\" : 0.0\n"
"				},\n"
"				\"Rotation\" : {\n"
"					\"x\" : 0.0,\n"
"					\"y\" : 0.62831852,\n"
"					\"z\" : 0.0\n"
"				},\n"
"				\"Scale\" : {\n"
"					\"x\" : 1.0,\n"
"					\"y\" : 1.0,\n"
"					\"z\" : 1.0\n"
"				}\n"
"			}\n"
"		},\n"
"		{\n"
"			\"ObjectType\" : \"Sphere\",\n"
"			\"UUID\" : \"938FC005-8455-4483-923A-7F1BDF3A9E57\",\n"
"			\"Name\" : \"sphere\",\n"
"			\"Transform\" : {\n"
"				\"Parent\" : \"E07B5012-320F-45B2-8EB0-6ADD00277038\",\n"
"				\"Translation\" : {\n"
"					\"x\" : 0.0,\n"
"					\"y\" : 2.5,\n"
"					\"z\" : -0.5\n"
"				},\n"
"				\"Rotation\" : {\n"
"					\"x\" : 0.0,\n"
"					\"y\" : 0.0,\n"
"					\"z\" : 0.0\n"
"				},\n"
"				\"Scale\" : {\n"
"					\"x\" : 1.0,\n"
"					\"y\" : 1.0,\n"
"					\"z\" : 1.0\n"
"				}\n"
"			}\n"
"		}\n"
"	]\n"
"}\n";

using namespace rapidjson;

namespace StenGine
{

class SceneFileManagerImpl : public SceneFileManager
{
public:
	void LoadScene(std::wstring path) final
	{
		Document sceneDoc;

		auto ret = sceneDoc.Parse(testScene).HasParseError();
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
	std::wstring mLoadedScene;
};

DEFINE_ABSTRACT_SINGLETON_CLASS(SceneFileManager, SceneFileManagerImpl);

}