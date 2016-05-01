#include "Engine/Console.h"
#include <Windows.h>
#include <ios>

#pragma warning(disable:4996)

namespace StenGine
{

Console::Console()
{
	/*******create console window**************/
	AllocConsole();

	// redirect unbuffered STDOUT to the console
	freopen("CONOUT$", "w+t", stdout);

	// redirect unbuffered STDIN to the console
	freopen("CONIN$", "r+t", stdin);

	// redirect unbuffered STDERR to the console
	freopen("CONOUT$", "w+t", stderr);

	std::ios::sync_with_stdio();

	/*********************************/
}

Console::~Console()
{

}

}