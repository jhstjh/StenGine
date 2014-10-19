#ifndef __OBJ_READER__
#define __OBJ_READER__

#include "D3DIncludes.h"
#include "MeshRenderer.h"

class ObjReader {
public:
	static void Read(const std::wstring& filename, Mesh* mesh);
};

#endif // !__OBJ_READER__
