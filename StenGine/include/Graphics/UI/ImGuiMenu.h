#pragma once

#include "Windows.h"

struct ImDrawData;

namespace StenGine
{

class ImGuiMenu 
{
public:
	virtual void RenderDrawLists(ImDrawData* draw_data) = 0;
	static ImGuiMenu* Create();
	static ImGuiMenu* Instance();
	static bool Created() { return _instance != nullptr; }
	static ImGuiMenu* _instance;

	virtual bool HandleMsg(UINT msg, WPARAM wParam, LPARAM lParam) = 0;
};

}