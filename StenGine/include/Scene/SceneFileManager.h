#pragma once
#include <string>
#include "System/SingletonClass.h"

namespace StenGine
{

class SceneFileManager : public AbstractSingletonClass<SceneFileManager>
{
public:
	virtual void LoadScene() = 0;
	virtual void Save() = 0;

	virtual void DrawMenu() = 0;

	DECLARE_ABSTRACT_SINGLETON_CLASS(SceneFileManager)
};

}