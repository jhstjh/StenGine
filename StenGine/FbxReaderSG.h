#ifndef __FBX_READER__
#define __FBX_READER__

#include "D3DIncludes.h"
#include "MeshRenderer.h"

class FbxReaderSG {
public:
	static bool Read(const std::wstring& filename, Mesh* mesh);
};

#endif // !__OBJ_READER__
