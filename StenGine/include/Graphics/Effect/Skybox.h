#ifndef __SKYBOX__
#define __SKYBOX__

#include "Graphics/D3DIncludes.h"

#ifdef GRAPHICS_OPENGL
#include "glew.h"
#endif

class Skybox {
public:
	Skybox(std::wstring &cubeMapPath);
	~Skybox();

	void Draw(); 
#ifdef GRAPHICS_D3D11
	ID3D11ShaderResourceView* m_cubeMapSRV;
#else
	uint64_t m_cubeMapTex;
	GLuint m_skyboxVAO;
#endif

};

#endif