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
	};
}

class MeshRenderer {
private:
	ID3D11Buffer* m_indexBufferGPU;
	ID3D11Buffer* m_stdMeshVertexBufferGPU;
	Effect* m_associatedEffect;

	void CreateBoxPrimitive();
	void PrepareGPUBuffer();

public:
	std::vector<UINT> m_indexBufferCPU;
	std::vector<XMFLOAT3> m_positionBufferCPU;
	std::vector<XMFLOAT3> m_normalBufferCPU;
	std::vector<XMFLOAT2> m_texUVBufferCPU;
	std::vector<XMFLOAT4> m_colorBufferCPU;

	MeshRenderer();
	~MeshRenderer();
	void Draw();

};

#endif // !__MESH_RENDERER__
