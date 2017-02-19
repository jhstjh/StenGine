#include <fstream>

#include "Mesh/Terrain.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/D3DIncludes.h"
#include "Graphics/Effect/ShadowMap.h"
#include "Scene/CameraManager.h"
#include "Scene/LightManager.h"
#include "Scene/GameObject.h"
#include "Resource/ResourceManager.h"
#include "Math/MathHelper.h"
#include "imgui.h"

#include "Graphics/OpenGL/GLImageLoader.h"

namespace StenGine
{

Terrain::Terrain(InitInfo &info) :
	m_quadPatchVB(0),
	m_quadPatchIB(0),
	m_layerMapArrayTex(0),
	m_blendMapTex(0),
	m_heightMapTex(0),
	m_numPatchVertices(0),
	m_numPatchQuadFaces(0),
	m_numPatchVertRows(0),
	m_numPatchVertCols(0),
	m_initInfo(info)
{
	m_numPatchVertRows = ((m_initInfo.HeightmapHeight - 1) / CellsPerPatch) + 1;
	m_numPatchVertCols = ((m_initInfo.HeightmapWidth - 1) / CellsPerPatch) + 1;

	m_numPatchVertices = m_numPatchVertRows * m_numPatchVertCols;
	m_numPatchQuadFaces = (m_numPatchVertRows - 1) * (m_numPatchVertCols - 1);

	LoadHeightmap();
	Smooth();

	CalcAllPatchBoundsY();

	BuildQuadPatchVB();
	BuildQuadPatchIB();
	BuildHeightMapSRV();

	m_blendMapTex = ResourceManager::Instance()->GetResource<Texture>(m_initInfo.BlendMapFilename);
	m_layerMapArrayTex = ResourceManager::Instance()->GetResource<Texture>(m_initInfo.LayerMapFilenames);
}

Terrain::~Terrain() {
	SafeDelete(m_heightMapTex);
	SafeDelete(m_quadPatchIB);
	SafeDelete(m_quadPatchVB);
}

void Terrain::CalcPatchBoundsY(UINT i, UINT j)
{
	// Scan the heightmap values this patch covers and compute the min/max height.

	UINT x0 = j*CellsPerPatch;
	UINT x1 = (j + 1)*CellsPerPatch;

	UINT y0 = i*CellsPerPatch;
	UINT y1 = (i + 1)*CellsPerPatch;

	float minY = FLT_MAX;
	float maxY = -FLT_MAX;
	for (UINT y = y0; y <= y1; ++y)
	{
		for (UINT x = x0; x <= x1; ++x)
		{
			UINT k = y*m_initInfo.HeightmapWidth + x;
			minY = min(minY, m_heightMap[k]);
			maxY = max(maxY, m_heightMap[k]);
		}
	}

	UINT patchID = i*(m_numPatchVertCols - 1) + j;
	m_patchBoundsY[patchID] = XMFLOAT2(minY, maxY);
}

void Terrain::CalcAllPatchBoundsY()
{
	m_patchBoundsY.resize(m_numPatchQuadFaces);

	// For each patch
	for (UINT i = 0; i < m_numPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < m_numPatchVertCols - 1; ++j)
		{
			CalcPatchBoundsY(i, j);
		}
	}
}

void Terrain::LoadHeightmap() {
	// A height for each vertex
	std::vector<unsigned char> in(m_initInfo.HeightmapWidth * m_initInfo.HeightmapHeight);

	// Open the file.
	std::ifstream inFile;
	inFile.open(m_initInfo.HeightMapFilename.c_str(), std::ios_base::binary);

	if (inFile) {
		// Read the RAW bytes.
		inFile.read((char*)&in[0], (std::streamsize)in.size());

		// Done with file.
		inFile.close();
	}

	// Copy the array data into a float array and scale it.
	m_heightMap.resize(m_initInfo.HeightmapHeight * m_initInfo.HeightmapWidth, 0);
	for (UINT i = 0; i < m_initInfo.HeightmapHeight * m_initInfo.HeightmapWidth; ++i) {
		m_heightMap[i] = (in[i] / 255.0f)*m_initInfo.HeightScale;
	}
}

void Terrain::Smooth()
{
	std::vector<float> dest(m_heightMap.size());

	for (UINT i = 0; i < m_initInfo.HeightmapHeight; ++i)
	{
		for (UINT j = 0; j < m_initInfo.HeightmapWidth; ++j)
		{
			dest[i*m_initInfo.HeightmapWidth + j] = Average(i, j);
		}
	}

	// Replace the old heightmap with the filtered one.
	m_heightMap = dest;
}

bool Terrain::InBounds(int i, int j)
{
	// True if ij are valid indices; false otherwise.
	return
		i >= 0 && i < (int)m_initInfo.HeightmapHeight &&
		j >= 0 && j < (int)m_initInfo.HeightmapWidth;
}

float Terrain::Average(int i, int j)
{
	// Function computes the average height of the ij element.
	// It averages itself with its eight neighbor pixels.  Note
	// that if a pixel is missing neighbor, we just don't include it
	// in the average--that is, edge pixels don't have a neighbor pixel.
	//
	// ----------
	// | 1| 2| 3|
	// ----------
	// |4 |ij| 6|
	// ----------
	// | 7| 8| 9|
	// ----------

	float avg = 0.0f;
	float num = 0.0f;

	// Use int to allow negatives.  If we use UINT, @ i=0, m=i-1=UINT_MAX
	// and no iterations of the outer for loop occur.
	for (int m = i - 1; m <= i + 1; ++m)
	{
		for (int n = j - 1; n <= j + 1; ++n)
		{
			if (InBounds(m, n))
			{
				avg += m_heightMap[m * m_initInfo.HeightmapWidth + n];
				num += 1.0f;
			}
		}
	}

	return avg / num;
}

void Terrain::BuildHeightMapSRV() {

	std::vector<HALF> hmap(m_heightMap.size());
	std::transform(m_heightMap.begin(), m_heightMap.end(), hmap.begin(), XMConvertFloatToHalf);

	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = m_initInfo.HeightmapWidth;
		texDesc.Height = m_initInfo.HeightmapHeight;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R16_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = &hmap[0];
		data.SysMemPitch = m_initInfo.HeightmapWidth * sizeof(HALF);
		data.SysMemSlicePitch = 0;

		ID3D11Texture2D* hmapTex = 0;
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateTexture2D(&texDesc, &data, &hmapTex));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;

		ID3D11ShaderResourceView* heightMapSRV;
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateShaderResourceView(hmapTex, &srvDesc, &heightMapSRV));

		m_heightMapTex = new Texture(m_initInfo.HeightmapWidth, m_initInfo.HeightmapHeight, heightMapSRV);

		// SRV saves reference.
		ReleaseCOM(hmapTex);
		break;
	}
	case RenderBackend::OPENGL4:
	{

		GLuint hmapTex;
		glCreateTextures(GL_TEXTURE_2D, 1, &hmapTex);
		glTextureStorage2D(hmapTex, 1, GL_R32F, m_initInfo.HeightmapWidth, m_initInfo.HeightmapHeight);

		GLuint pbo;
		glCreateBuffers(1, &pbo);
		glNamedBufferData(pbo, sizeof(m_heightMap[0]) * m_heightMap.size(), nullptr, GL_STREAM_DRAW);

		void* pboData = glMapNamedBuffer(pbo, GL_WRITE_ONLY);
		memcpy(pboData, &m_heightMap[0], sizeof(m_heightMap[0]) * m_heightMap.size());
		glUnmapNamedBuffer(pbo);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		glTextureSubImage2D(hmapTex, 0, 0, 0, m_initInfo.HeightmapWidth, m_initInfo.HeightmapHeight, GL_RED, GL_FLOAT, 0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		glDeleteBuffers(1, &pbo);

		m_heightMapTex = new Texture(m_initInfo.HeightmapWidth, m_initInfo.HeightmapHeight, reinterpret_cast<void*>(hmapTex));
		break;
	}
	}
}

float Terrain::GetWidth() const {
	return (m_initInfo.HeightmapWidth - 1) * m_initInfo.CellSpacing;
}

float Terrain::GetDepth() const {
	return (m_initInfo.HeightmapHeight - 1) * m_initInfo.CellSpacing;
}

float Terrain::GetHeight(float x, float z) const {

	return 0;
}

void Terrain::BuildQuadPatchVB() {
	std::vector<Vertex::TerrainVertex> patchVertices(m_numPatchVertRows * m_numPatchVertCols);

	float halfWidth = 0.5f * GetWidth();
	float halfDepth = 0.5f * GetDepth();

	float patchWidth = GetWidth() / (m_numPatchVertCols - 1);
	float patchDepth = GetDepth() / (m_numPatchVertRows - 1);
	float du = 1.0f / (m_numPatchVertCols - 1);
	float dv = 1.0f / (m_numPatchVertRows - 1);

	for (UINT i = 0; i < m_numPatchVertRows; ++i) {
		float z = halfDepth - i * patchDepth;
		for (UINT j = 0; j < m_numPatchVertCols; ++j) {
			float x = -halfWidth + j * patchWidth;

			patchVertices[i * m_numPatchVertCols + j].Pos = XMFLOAT3(x, 0.f, z);

			patchVertices[i * m_numPatchVertCols + j].TexUV.x = j * du;
			patchVertices[i * m_numPatchVertCols + j].TexUV.y = i * dv;
		}
	}

	for (UINT i = 0; i < m_numPatchVertRows - 1; ++i) {
		for (UINT j = 0; j < m_numPatchVertCols - 1; ++j) {
			UINT patchID = i * (m_numPatchVertCols - 1) + j;
			patchVertices[i * m_numPatchVertCols + j].BoundsY = m_patchBoundsY[patchID];
		}
	}

	m_quadPatchVB = new GPUBuffer(patchVertices.size() * sizeof(Vertex::TerrainVertex), BufferUsage::IMMUTABLE, (void*)&patchVertices.front(), BufferType::VERTEX_BUFFER);
}

void Terrain::BuildQuadPatchIB() {
	std::vector<UINT> indices(m_numPatchQuadFaces * 4);

	int k = 0;
	for (UINT i = 0; i < m_numPatchVertRows - 1; ++i) {
		for (UINT j = 0; j < m_numPatchVertCols - 1; ++j) {
			indices[k] = i * m_numPatchVertCols + j;
			indices[k + 1] = i * m_numPatchVertCols + j + 1;

			indices[k + 2] = (i + 1) * m_numPatchVertCols + j;
			indices[k + 3] = (i + 1) * m_numPatchVertCols + j + 1;

			k += 4;
		}
	}
	m_quadPatchIB = new GPUBuffer(indices.size() * sizeof(UINT), BufferUsage::IMMUTABLE, (void*)&indices.front(), BufferType::INDEX_BUFFER);
}

void Terrain::GatherDrawCall() 
{
	UINT stride = sizeof(Vertex::TerrainVertex);
	UINT offset = 0;

	DeferredGeometryTerrainPassEffect* effect = EffectsManager::Instance()->m_deferredGeometryTerrainPassEffect.get();

	ConstantBuffer cbuffer0(1, sizeof(DeferredGeometryTerrainPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
	ConstantBuffer cbuffer1(0, sizeof(DeferredGeometryTerrainPassEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

	DeferredGeometryTerrainPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredGeometryTerrainPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0.GetBuffer();
	DeferredGeometryTerrainPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredGeometryTerrainPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1.GetBuffer();

	DrawCmd cmd;

	perframeData->gEyePosW = XMFLOAT4(&CameraManager::Instance()->GetActiveCamera()->GetPos()[0]);

	perframeData->gMaxDist = 500.00;
	perframeData->gMinDist = 20;
	perframeData->gMaxTess = 6.f;
	perframeData->gMinTess = 0.f;
	perframeData->gTexelCellSpaceU = 1.0f / m_initInfo.HeightmapWidth;
	perframeData->gTexelCellSpaceV = 1.0f / m_initInfo.HeightmapHeight;
	perframeData->gTexScale = XMFLOAT2(50.f, 50.f);
	perframeData->gWorldCellSpace = m_initInfo.CellSpacing;
	perframeData->gWorldFrustumPlanes /********************/;

	perObjData->View = TRASNPOSE_API_CHOOSER(XMMATRIX(&CameraManager::Instance()->GetActiveCamera()->GetViewMatrix()[0]));
	perObjData->ViewProj = TRASNPOSE_API_CHOOSER(XMMATRIX(&CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix()[0]));
	perObjData->World = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[0]->GetTransform()->GetWorldTransform()));
	perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(MatrixHelper::InverseTranspose(XMLoadFloat4x4(m_parents[0]->GetTransform()->GetWorldTransform())));

	XMMATRIX worldView = XMLoadFloat4x4(m_parents[0]->GetTransform()->GetWorldTransform()) * XMMATRIX(&CameraManager::Instance()->GetActiveCamera()->GetViewMatrix()[0]);
	perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);

	XMMATRIX worldViewInvTranspose = MatrixHelper::InverseTranspose(worldView);
	perObjData->WorldViewInvTranspose = TRASNPOSE_API_CHOOSER(worldViewInvTranspose);

	perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetShadowMapTransform());
	perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[0]->GetTransform()->GetWorldTransform()) * XMMATRIX(&CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix()[0]));

	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(LightManager::Instance()->m_shadowMap->GetDepthSRV()), 3);
		cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(m_heightMapTex->GetTexture()), 5);
		cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(m_layerMapArrayTex->GetTexture()), 6);
		cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(m_blendMapTex->GetTexture()), 7);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		ConstantBuffer cbuffer2(2, sizeof(DeferredGeometryTerrainPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), effect->m_textureCB);
		DeferredGeometryTerrainPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (DeferredGeometryTerrainPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer2.GetBuffer();

		textureData->gShadowMap = LightManager::Instance()->m_shadowMap->GetDepthTexHandle();
		textureData->gHeightMap = reinterpret_cast<uint64_t>(m_heightMapTex->GetTexture());
		textureData->gLayerMapArray = reinterpret_cast<uint64_t>(m_layerMapArrayTex->GetTexture());
		textureData->gBlendMap = reinterpret_cast<uint64_t>(m_blendMapTex->GetTexture());

		cmd.cbuffers.push_back(std::move(cbuffer2));
		break;
	}
	}

	cmd.drawType = DrawType::INDEXED;
	cmd.flags = CmdFlag::DRAW;
	cmd.offset = (void*)(0);
	cmd.type = PrimitiveTopology::CONTROL_POINT_4_PATCHLIST;
	cmd.inputLayout = effect->GetInputLayout();
	cmd.framebuffer = &Renderer::Instance()->GetGbuffer();
	cmd.vertexBuffer = (void*)m_quadPatchVB->GetBuffer();
	cmd.indexBuffer = (void*)m_quadPatchIB->GetBuffer();
	cmd.vertexStride = stride;
	cmd.vertexOffset = offset;
	cmd.effect = effect;
	cmd.elementCount = m_numPatchQuadFaces * 4;
	cmd.cbuffers.push_back(std::move(cbuffer0));
	cmd.cbuffers.push_back(std::move(cbuffer1));

	Renderer::Instance()->AddDeferredDrawCmd(cmd);
}

void Terrain::GatherShadowDrawCall() {

	UINT stride = sizeof(Vertex::TerrainVertex);
	UINT offset = 0;

	TerrainShadowMapEffect* effect = EffectsManager::Instance()->m_terrainShadowMapEffect.get();

	ConstantBuffer cbuffer0(1, sizeof(TerrainShadowMapEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
	ConstantBuffer cbuffer1(0, sizeof(TerrainShadowMapEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

	TerrainShadowMapEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (TerrainShadowMapEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0.GetBuffer();
	TerrainShadowMapEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (TerrainShadowMapEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1.GetBuffer();

	DrawCmd cmd;

	perframeData->gEyePosW = XMFLOAT4(&CameraManager::Instance()->GetActiveCamera()->GetPos()[0]);

	perframeData->gMaxDist = 500.00;
	perframeData->gMinDist = 20;
	perframeData->gMaxTess = 6.f;
	perframeData->gMinTess = 0.f;
	perframeData->gTexelCellSpaceU = 1.0f / m_initInfo.HeightmapWidth;
	perframeData->gTexelCellSpaceV = 1.0f / m_initInfo.HeightmapHeight;
	perframeData->gTexScale = XMFLOAT2(50.f, 50.f);
	perframeData->gWorldCellSpace = m_initInfo.CellSpacing;
	perframeData->gWorldFrustumPlanes /********************/;

	perObjData->View = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetViewMatrix());
	perObjData->ViewProj = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetViewProjMatrix());
	perObjData->World = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[0]->GetTransform()->GetWorldTransform()));
	perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(MatrixHelper::InverseTranspose(XMLoadFloat4x4(m_parents[0]->GetTransform()->GetWorldTransform())));

	XMFLOAT4 resourceMask(0, 0, 0, 0);

	XMMATRIX worldView = XMLoadFloat4x4(m_parents[0]->GetTransform()->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetViewMatrix();
	perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);

	XMMATRIX worldViewInvTranspose = MatrixHelper::InverseTranspose(worldView);
	perObjData->WorldViewInvTranspose = TRASNPOSE_API_CHOOSER(worldViewInvTranspose);

	//perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetShadowMapTransform());
	perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(XMLoadFloat4x4(m_parents[0]->GetTransform()->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetViewProjMatrix());

	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(LightManager::Instance()->m_shadowMap->GetDepthSRV()), 3);
		cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(m_heightMapTex->GetTexture()), 5);
		cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(m_layerMapArrayTex->GetTexture()), 6);
		cmd.srvs.AddSRV(reinterpret_cast<ID3D11ShaderResourceView*>(m_blendMapTex->GetTexture()), 7);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		ConstantBuffer cbuffer2(2, sizeof(TerrainShadowMapEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), effect->m_textureCB);
		TerrainShadowMapEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (TerrainShadowMapEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer2.GetBuffer();

		textureData->gShadowMap = LightManager::Instance()->m_shadowMap->GetDepthTex();
		textureData->gHeightMap = reinterpret_cast<uint64_t>(m_heightMapTex->GetTexture());
		textureData->gLayerMapArray = reinterpret_cast<uint64_t>(m_layerMapArrayTex->GetTexture());
		textureData->gBlendMap = reinterpret_cast<uint64_t>(m_blendMapTex->GetTexture());

		cmd.cbuffers.push_back(std::move(cbuffer2));

		break;
	}
	}

	cmd.drawType = DrawType::INDEXED;
	cmd.flags = CmdFlag::DRAW;
	cmd.offset = (void*)(0);
	cmd.type = PrimitiveTopology::CONTROL_POINT_4_PATCHLIST;
	cmd.inputLayout = effect->GetInputLayout();
	cmd.framebuffer = &Renderer::Instance()->GetGbuffer();
	cmd.vertexBuffer = (void*)m_quadPatchVB->GetBuffer();
	cmd.indexBuffer = (void*)m_quadPatchIB->GetBuffer();
	cmd.vertexStride = stride;
	cmd.vertexOffset = offset;
	cmd.effect = effect;
	cmd.elementCount = m_numPatchQuadFaces * 4;
	cmd.cbuffers.push_back(std::move(cbuffer0));
	cmd.cbuffers.push_back(std::move(cbuffer1));

	Renderer::Instance()->AddDeferredDrawCmd(cmd);

}

void Terrain::DrawMenu()
{
	if (ImGui::CollapsingHeader("Terrain Renderer", nullptr, true, true))
	{

	}
}

}