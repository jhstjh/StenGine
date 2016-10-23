#ifndef __SKYBOX__
#define __SKYBOX__

#include "Graphics/D3DIncludes.h"
#include "glew.h"

namespace StenGine
{

class Skybox {
public:
	Skybox(std::wstring &cubeMapPath);
	~Skybox();

	void Draw();

	// TODO unify D3D/GL member
	ID3D11ShaderResourceView* m_cubeMapSRV;

	uint64_t m_cubeMapTex;
	GLuint m_skyboxVAO;
};

}
#endif