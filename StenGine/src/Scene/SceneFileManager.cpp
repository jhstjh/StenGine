#include "tinyxml2.h"
#include "imgui.h"
#include "Scene/GameObjectManager.h"
#include "Scene/Savable.h"
#include "Scene/SceneFileManager.h"
#include <Windows.h>
#include <chrono>
#include <ctime>

namespace StenGine
{

class SceneFileManagerImpl : public SceneFileManager
{
public:
	void LoadScene() final
	{
		OPENFILENAME ofn;
		wchar_t szFileName[MAX_PATH] = L"";
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFilter = (LPCWSTR)L"StenGine Scene File (*.sgs)\0All Files (*.*)\0*.*\0";
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
		ofn.lpstrDefExt = (LPCWSTR)L"sgs";

		if (GetOpenFileName(&ofn))
		{
			tinyxml2::XMLDocument save;

			tinyxml2::XMLError eResult = save.LoadFile("SavedData.xml");
		}
	}

	void Save() final
	{
		OPENFILENAME ofn;
		wchar_t szFileName[MAX_PATH] = L"New Scene";
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFilter = (LPCWSTR)L"StenGine Scene File (*.sgs)\0All Files (*.*)\0*.*\0";
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER;
		ofn.lpstrDefExt = (LPCWSTR)L"sgs";

		if (GetSaveFileName(&ofn))
		{
			FILE* fp = _wfopen(ofn.lpstrFile, L"w");
			assert(fp);

			tinyxml2::XMLDocument save;

			tinyxml2::XMLElement* pHeader = save.NewElement("Header");
			pHeader->SetAttribute("version", "1.0");

			auto now = std::chrono::system_clock::now();
			std::time_t now_time = std::chrono::system_clock::to_time_t(now);
			pHeader->SetAttribute("timestamp", std::ctime(&now_time));

			save.InsertFirstChild(pHeader);

			tinyxml2::XMLElement* pRoot = save.NewElement("SceneRoot");
			save.InsertEndChild(pRoot);

			const auto &gameObjects = GameObjectManager::Instance()->GetAllGameObjects();
			pRoot->SetAttribute("count", static_cast<uint32_t>(gameObjects.size()));

			for(auto &gameObject: gameObjects)
			{
				tinyxml2::XMLElement* pGameObject = save.NewElement("GameObject");
				auto uuid = gameObject->GetUUID();

				const char* uuid_str = "";
				RPC_CSTR szUuid = NULL;
				if (UuidToStringA(&uuid, &szUuid) == RPC_S_OK)
				{
					uuid_str = (char*)szUuid;
					RpcStringFreeA(&szUuid);
				}

				pGameObject->SetAttribute("uuid", uuid_str);
				pGameObject->SetAttribute("name", gameObject->GetName().c_str());
				pGameObject->SetAttribute("type", gameObject->GetType());

				const auto components = gameObject->GetAllComponents();
				for (auto &component : components)
				{
					const Savable* savable = dynamic_cast<const Savable*>(component);
					if (savable)
					{
						auto ret = savable->Save(save);
						if (ret)
						{
							pGameObject->InsertEndChild(ret);
						}
					}
				}

				pRoot->InsertEndChild(pGameObject);
			}		

			tinyxml2::XMLError eResult = save.SaveFile(fp);

			fclose(fp);
		}
	}
	
	void DrawMenu() final
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Load"))
				{
					LoadScene();
				}
				if (ImGui::MenuItem("Save"))
				{
					Save();
				}
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