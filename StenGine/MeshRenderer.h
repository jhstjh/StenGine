#ifndef __MESH_RENDERER__
#define __MESH_RENDERER__
#include <vector>
#include "D3DIncludes.h"
#include "EffectsManager.h"

class Effect;

namespace Vertex {
	struct StdMeshVertex {
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};
}

class MeshRenderer {
private:
	std::vector<UINT> m_indexBufferCPU;
	std::vector<XMFLOAT3> m_positionBufferCPU;
	std::vector<XMFLOAT3> m_normalBufferCPU;
	std::vector<XMFLOAT3> m_texUVBufferCPU;
	std::vector<XMFLOAT4> m_colorBufferCPU;

	ID3D11Buffer* m_indexBufferGPU;
	ID3D11Buffer* m_positionBufferGPU;
	ID3D11Buffer* m_colorBufferGPU;
	ID3D11Buffer* m_Pos_Color_VertexBufferGPU;
	Effect* m_associatedEffect;

	void CreateBoxPrimitive();
	void PrepareGPUBuffer();

public:
	MeshRenderer();
	~MeshRenderer();
	void Draw();

};

#endif // !__MESH_RENDERER__
