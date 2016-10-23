// StenGine.cpp : Defines the entry point for the application.
//
#include <stdio.h>
#include "windowsX.h"
#include "stdafx.h"
#include "StenGine.h"
#include "Utility/CommandlineParser.h"

#include "Engine/EngineBase.h"

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	std::unique_ptr<StenGine::CommandlineParser> cmdParser = std::unique_ptr<StenGine::CommandlineParser>(StenGine::CommandlineParser::Instance());
	cmdParser->Init(lpCmdLine);

	std::unique_ptr<StenGine::EngineBase> engine = std::make_unique<StenGine::EngineBase>();
	
	engine->Init(hInstance);
	engine->Run();

	return 0;
}

