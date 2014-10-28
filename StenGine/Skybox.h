#ifndef __SKYBOX__
#define __SKYBOX__
#include "D3DIncludes.h"
#include "MeshRenderer.h"

class Skybox {
public:
	Skybox(std::wstring &cubeMapPath);
	~Skybox();

	void Draw(); 
	ID3D11ShaderResourceView* m_cubeMapSRV;
};

#endif