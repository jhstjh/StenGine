#ifndef __SUB_MESH__
#define __SUB_MESH__

#include "Component.h"
#include "Material.h"
#include "D3DIncludes.h"

class SubMesh : public Component {
public:
	//SubMesh();
	//~SubMesh();
	void PrepareGPUBuffer();

	std::vector<UINT> m_indexBufferCPU;
	ID3D11Buffer* m_indexBufferGPU;
	ID3D11ShaderResourceView* m_diffuseMapSRV;
	ID3D11ShaderResourceView* m_normalMapSRV;
	ID3D11ShaderResourceView* m_bumpMapSRV;

	Material m_material;
};


#endif