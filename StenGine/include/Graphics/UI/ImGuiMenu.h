#pragma once

#include "System/SingletonClass.h"

struct ImDrawData;

namespace StenGine
{

class ImGuiMenu : public AbstractSingletonClass<ImGuiMenu>
{
public:
	virtual void RenderDrawLists(ImDrawData* draw_data) = 0;
	static ImGuiMenu* Instance();
};

}