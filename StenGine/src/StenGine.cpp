// StenGine.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <stdio.h>
#include "windowsX.h"
#include "stdafx.h"
#include "StenGine.h"
#include "Utility/CommandlineParser.h"

#include "Engine/EngineBase.h"

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

extern std::unique_ptr<StenGine::EngineBase> CreateGame();

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int /*nCmdShow*/)
{
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_WNDW);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	std::unique_ptr<StenGine::CommandlineParser> cmdParser = std::unique_ptr<StenGine::CommandlineParser>(StenGine::CommandlineParser::Create());
	cmdParser->Init(lpCmdLine);

	std::unique_ptr<StenGine::EngineBase> game = CreateGame();
	
	game->Init(hInstance);
	game->Run();

	return 0;
}

