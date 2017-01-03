#include "tinyxml2.h"
#include "imgui.h"
#include "Scene/SceneFileManager.h"

namespace StenGine
{

class SceneFileManagerImpl : public SceneFileManager
{
public:
	void LoadScene(std::wstring path) final
	{

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