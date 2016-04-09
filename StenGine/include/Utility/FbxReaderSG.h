#ifndef __FBX_READER__
#define __FBX_READER__

#if  (PLATFORM_WIN32) || defined(SG_TOOL)
#include "Graphics/D3DIncludes.h"
#endif
#include "Mesh/MeshRenderer.h"

class FbxReaderSG {
public:
	static bool Read(const std::wstring& filename, Mesh* mesh);
	static bool Read(const std::string& filename, Mesh* mesh);
};

#endif // !__OBJ_READER__
