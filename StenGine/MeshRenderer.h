#ifndef __MESH_RENDERER__
#define __MESH_RENDERER__
#include <vector>
#include "D3DIncludes.h"
#include "EffectsManager.h"

class Effect;

namespace Vertex {
	struct StdMeshVertex {
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 TexUV;
	};
}

struct Material {
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
};

class MeshRenderer {
private:
	ID3D11Buffer* m_indexBufferGPU;
	ID3D11Buffer* m_stdMeshVertexBufferGPU;
	Effect* m_associatedEffect;
	Material m_material;
	ID3D11ShaderResourceView* m_diffuseMapSRV;

	void CreateBoxPrimitive();
	void CreatePlanePrimitive();
	void PrepareGPUBuffer();
	void PrepareSRV(int type);

public:
	std::vector<UINT> m_indexBufferCPU;
	std::vector<XMFLOAT3> m_positionBufferCPU;
	std::vector<XMFLOAT3> m_normalBufferCPU;
	std::vector<XMFLOAT2> m_texUVBufferCPU;
	std::vector<XMFLOAT4> m_colorBufferCPU;

	XMFLOAT4X4 m_worldTransform;
	MeshRenderer(int type);
	~MeshRenderer();
	void Draw();

};

#endif // !__MESH_RENDERER__
