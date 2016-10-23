#ifndef __FBX_READER__
#define __FBX_READER__

#include "Graphics/D3DIncludes.h"
#include "Mesh/MeshRenderer.h"

namespace StenGine
{

class FbxReaderSG {
public:
	static bool Read(const std::wstring& filename, Mesh* mesh);
	static bool Read(const std::wstring& filename, class Animation* mesh);
	static bool Read(const std::string& filename, Mesh* mesh);
};

}

#endif // !__OBJ_READER__
