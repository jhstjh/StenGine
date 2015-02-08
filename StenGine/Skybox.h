#ifndef __SKYBOX__
#define __SKYBOX__
#include "D3DIncludes.h"
//#include "MeshRenderer.h"
#include "GL/glew.h"

class Skybox {
public:
	Skybox(std::wstring &cubeMapPath);
	~Skybox();

	void Draw(); 
#ifdef GRAPHICS_D3D11
	ID3D11ShaderResourceView* m_cubeMapSRV;
#else
	GLuint m_cubeMapTex;
#endif

	class hahaha* haha;
};

#endif