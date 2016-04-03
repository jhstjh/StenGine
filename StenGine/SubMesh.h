#ifndef __SUB_MESH__
#define __SUB_MESH__

#include "Component.h"
#include "Material.h"

#if defined(PLATFORM_WIN32) || defined(SG_TOOL)
#include "D3DIncludes.h"
#include "GL/glew.h"
#endif

class SubMesh : public Component {
public:
	SubMesh();
	//~SubMesh();
	void PrepareGPUBuffer();

	std::vector<UINT> m_indexBufferCPU;

#ifdef GRAPHICS_D3D11
	ID3D11Buffer* m_indexBufferGPU;
	ID3D11ShaderResourceView* m_diffuseMapSRV;
	ID3D11ShaderResourceView* m_normalMapSRV;
	ID3D11ShaderResourceView* m_bumpMapSRV;
#else
	GLuint m_indexBufferGPU;
	uint64_t m_diffuseMapTex;
	uint64_t m_normalMapTex;
	uint64_t m_bumpMapTex;
#endif

	Material m_material;
};


#endif