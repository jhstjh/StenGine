#pragma once

#include <string>
#include "MeshRenderer.h"

class SgmReader {
public:
	//static bool Read(const std::wstring& filename, Mesh* mesh);
	static bool Read(const std::string& filename, Mesh* mesh);
};