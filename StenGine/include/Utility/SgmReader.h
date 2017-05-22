#pragma once

#include <string>
#include "Mesh/MeshRenderer.h"

namespace StenGine
{

class SgmReader {
public:
	//static bool Read(const std::wstring& filename, Mesh* mesh);
	static bool Read(const std::string& filename, MeshRenderer* mesh);
};

}