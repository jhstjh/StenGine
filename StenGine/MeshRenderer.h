#ifndef __MESH_RENDERER__
#define __MESH_RENDERER__
#include <vector>
#include "D3DIncludes.h"
#include "EffectsManager.h"
#include "Component.h"

class Effect;

namespace Vertex {
	struct StdMeshVertex {
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT3 Tangent;
		XMFLOAT2 TexUV;
	};

	struct ShadowMapVertex {
		XMFLOAT3 Pos;
	};
}

struct Material {
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
};

class Mesh: public Component {
private:
	ID3D11Buffer* m_indexBufferGPU;
	ID3D11Buffer* m_stdMeshVertexBufferGPU;
	ID3D11Buffer* m_shadowMapVertexBufferGPU;
	Effect* m_associatedEffect;
	Effect* m_associatedDeferredEffect;


	void CreateBoxPrimitive();
	void CreatePlanePrimitive();
	void PrepareGPUBuffer();
	void PrepareShadowMapBuffer();
	void PrepareSRV(int type);

public:
	std::vector<UINT> m_indexBufferCPU;
	std::vector<XMFLOAT3> m_positionBufferCPU;
	std::vector<XMFLOAT3> m_normalBufferCPU;
	std::vector<XMFLOAT2> m_texUVBufferCPU;
	std::vector<XMFLOAT4> m_colorBufferCPU;
	std::vector<XMFLOAT3> m_tangentBufferCPU;
	ID3D11ShaderResourceView* m_diffuseMapSRV;
	ID3D11ShaderResourceView* m_normalMapSRV;

	Material m_material;
	XMFLOAT4X4 m_worldTransform;
	bool m_castShadow;
	bool m_receiveShadow;
	Mesh(int type);
	~Mesh();
	void Draw();
	void DrawOnShadowMap();
	void Prepare();


};

#endif // !__MESH_RENDERER__
