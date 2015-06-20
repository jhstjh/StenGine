#ifndef __FBX_READER__
#define __FBX_READER__

#if defined (PLATFORM_WIN32) || defined(SG_TOOL)
#include "D3DIncludes.h"
#endif
#include "MeshRenderer.h"

class FbxReaderSG {
public:
	static bool Read(const std::wstring& filename, Mesh* mesh);
	static bool Read(const std::string& filename, Mesh* mesh);
};

#endif // !__OBJ_READER__
