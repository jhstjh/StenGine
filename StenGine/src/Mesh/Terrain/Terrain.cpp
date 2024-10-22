#include "stdafx.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <imgui.h>

#include "Math/MathHelper.h"
#include "Mesh/Terrain/Terrain.h"
#include "Mesh/Terrain/TerrainGrass.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/D3DIncludes.h"
#include "Graphics/Effect/ShadowMap.h"
#include "Graphics/OpenGL/GLImageLoader.h"
#include "Scene/CameraManager.h"
#include "Scene/LightManager.h"
#include "Scene/GameObject.h"
#include "Resource/ResourceManager.h"


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

	m_blendMapTex = ResourceManager::Instance()->GetSharedResource<Texture>(m_initInfo.BlendMapFilename);
	m_layerMapArrayTex = ResourceManager::Instance()->GetSharedResource<Texture>(m_initInfo.LayerMapFilenames);

	mGrasses.emplace_back(this, GetWidth(), GetDepth());
}

Terrain::~Terrain() 
{

}

void Terrain::CalcPatchBoundsY(uint32_t i, uint32_t j)
{
	// Scan the heightmap values this patch covers and compute the min/max height.

	uint32_t x0 = j*CellsPerPatch;
	uint32_t x1 = (j + 1)*CellsPerPatch;

	uint32_t y0 = i*CellsPerPatch;
	uint32_t y1 = (i + 1)*CellsPerPatch;

	float minY = FLT_MAX;
	float maxY = -FLT_MAX;
	for (uint32_t y = y0; y <= y1; ++y)
	{
		for (uint32_t x = x0; x <= x1; ++x)
		{
			uint32_t k = y*m_initInfo.HeightmapWidth + x;
			minY = std::min(minY, m_heightMap[k]);
			maxY = std::max(maxY, m_heightMap[k]);
		}
	}

	uint32_t patchID = i*(m_numPatchVertCols - 1) + j;
	m_patchBoundsY[patchID] = Vec2(minY, maxY);
}

void Terrain::CalcAllPatchBoundsY()
{
	m_patchBoundsY.resize(m_numPatchQuadFaces);

	// For each patch
	for (uint32_t i = 0; i < m_numPatchVertRows - 1; ++i)
	{
		for (uint32_t j = 0; j < m_numPatchVertCols - 1; ++j)
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
	for (uint32_t i = 0; i < m_initInfo.HeightmapHeight * m_initInfo.HeightmapWidth; ++i) {
		m_heightMap[i] = (in[i] / 255.0f)*m_initInfo.HeightScale;
	}
}

void Terrain::Smooth()
{
	std::vector<float> dest(m_heightMap.size());

	for (uint32_t i = 0; i < m_initInfo.HeightmapHeight; ++i)
	{
		for (uint32_t j = 0; j < m_initInfo.HeightmapWidth; ++j)
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

	// Use int to allow negatives.  If we use uint32_t, @ i=0, m=i-1=uint32_t_MAX
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

	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = m_initInfo.HeightmapWidth;
		texDesc.Height = m_initInfo.HeightmapHeight;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = &m_heightMap[0];
		data.SysMemPitch = m_initInfo.HeightmapWidth * sizeof(float);
		data.SysMemSlicePitch = 0;

		ID3D11Texture2D* hmapTex = 0;
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateTexture2D(&texDesc, &data, &hmapTex));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		ID3D11ShaderResourceView* heightMapSRV;
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateShaderResourceView(hmapTex, &srvDesc, &heightMapSRV));

		m_heightMapTex = Renderer::Instance()->CreateTexture(m_initInfo.HeightmapWidth, m_initInfo.HeightmapHeight, heightMapSRV);

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

		m_heightMapTex = Renderer::Instance()->CreateTexture(m_initInfo.HeightmapWidth, m_initInfo.HeightmapHeight, reinterpret_cast<void*>(hmapTex));
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

	// Transform from terrain local space to "cell" space.
	float c = (x + 0.5f*GetWidth()) / m_initInfo.CellSpacing;
	float d = (z - 0.5f*GetDepth()) / -m_initInfo.CellSpacing;

	// Get the row and column we are in.
	int row = (int)floorf(d);
	int col = (int)floorf(c);

	// Grab the heights of the cell we are in.
	// A*--*B
	//  | /|
	//  |/ |
	// C*--*D
	float A = m_heightMap[row*m_initInfo.HeightmapWidth + col];
	float B = m_heightMap[row*m_initInfo.HeightmapWidth + col + 1];
	float C = m_heightMap[(row + 1)*m_initInfo.HeightmapWidth + col];
	float D = m_heightMap[(row + 1)*m_initInfo.HeightmapWidth + col + 1];

	// Where we are relative to the cell.
	float s = c - (float)col;
	float t = d - (float)row;

	// If upper triangle ABC.
	if (s + t <= 1.0f)
	{
		float uy = B - A;
		float vy = C - A;
		return A + s * uy + t * vy;
	}
	else // lower triangle DCB.
	{
		float uy = C - D;
		float vy = B - D;
		return D + (1.0f - s)*uy + (1.0f - t)*vy;
	}
}

void Terrain::BuildQuadPatchVB() {
	std::vector<Vertex::TerrainVertex> patchVertices(m_numPatchVertRows * m_numPatchVertCols);

	float halfWidth = 0.5f * GetWidth();
	float halfDepth = 0.5f * GetDepth();

	float patchWidth = GetWidth() / (m_numPatchVertCols - 1);
	float patchDepth = GetDepth() / (m_numPatchVertRows - 1);
	float du = 1.0f / (m_numPatchVertCols - 1);
	float dv = 1.0f / (m_numPatchVertRows - 1);

	for (uint32_t i = 0; i < m_numPatchVertRows; ++i) {
		float z = halfDepth - i * patchDepth;
		for (uint32_t j = 0; j < m_numPatchVertCols; ++j) {
			float x = -halfWidth + j * patchWidth;

			patchVertices[i * m_numPatchVertCols + j].Pos = Vec3(x, 0.f, z);

			patchVertices[i * m_numPatchVertCols + j].TexUV.data[0] = j * du;
			patchVertices[i * m_numPatchVertCols + j].TexUV.data[1] = i * dv;
		}
	}

	for (uint32_t i = 0; i < m_numPatchVertRows - 1; ++i) {
		for (uint32_t j = 0; j < m_numPatchVertCols - 1; ++j) {
			uint32_t patchID = i * (m_numPatchVertCols - 1) + j;
			patchVertices[i * m_numPatchVertCols + j].BoundsY = m_patchBoundsY[patchID];
		}
	}

	m_quadPatchVB= Renderer::Instance()->CreateGPUBuffer(patchVertices.size() * sizeof(Vertex::TerrainVertex), BufferUsage::IMMUTABLE, (void*)&patchVertices.front(), BufferType::VERTEX_BUFFER);
}

void Terrain::BuildQuadPatchIB() {
	std::vector<uint32_t> indices(m_numPatchQuadFaces * 4);

	int k = 0;
	for (uint32_t i = 0; i < m_numPatchVertRows - 1; ++i) {
		for (uint32_t j = 0; j < m_numPatchVertCols - 1; ++j) {
			indices[k] = i * m_numPatchVertCols + j;
			indices[k + 1] = i * m_numPatchVertCols + j + 1;

			indices[k + 2] = (i + 1) * m_numPatchVertCols + j;
			indices[k + 3] = (i + 1) * m_numPatchVertCols + j + 1;

			k += 4;
		}
	}
	m_quadPatchIB= Renderer::Instance()->CreateGPUBuffer(indices.size() * sizeof(uint32_t), BufferUsage::IMMUTABLE, (void*)&indices.front(), BufferType::INDEX_BUFFER);
}

void Terrain::GatherDrawCall() 
{
	uint32_t stride = sizeof(Vertex::TerrainVertex);
	uint32_t offset = 0;

	DeferredGeometryTerrainPassEffect* effect = EffectsManager::Instance()->m_deferredGeometryTerrainPassEffect.get();

	ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(1, sizeof(DeferredGeometryTerrainPassEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
	ConstantBuffer cbuffer1 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(DeferredGeometryTerrainPassEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

	DeferredGeometryTerrainPassEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (DeferredGeometryTerrainPassEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0->GetBuffer();
	DeferredGeometryTerrainPassEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (DeferredGeometryTerrainPassEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1->GetBuffer();

	DrawCmd cmd;

	perframeData->gEyePosW = CameraManager::Instance()->GetActiveCamera()->GetPos();

	perframeData->gMaxDist = 500.00;
	perframeData->gMinDist = 20;
	perframeData->gMaxTess = 6.f;
	perframeData->gMinTess = 0.f;
	perframeData->gTexelCellSpaceU = 1.0f / m_initInfo.HeightmapWidth;
	perframeData->gTexelCellSpaceV = 1.0f / m_initInfo.HeightmapHeight;
	perframeData->gTexScale = Vec2(50.f, 50.f);
	perframeData->gWorldCellSpace = m_initInfo.CellSpacing;
	perframeData->gWorldFrustumPlanes /********************/;

	perObjData->View = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewMatrix());
	perObjData->ViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());
	perObjData->World = TRASNPOSE_API_CHOOSER(mParent->GetTransform()->GetWorldTransform());
	perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(mParent->GetTransform()->GetWorldTransformInversed().Transpose());

	Mat4 worldView = CameraManager::Instance()->GetActiveCamera()->GetViewMatrix() * mParent->GetTransform()->GetWorldTransform();
	perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);

	Mat4 worldViewInvTranspose = worldView.Inverse().Transpose();
	perObjData->WorldViewInvTranspose = TRASNPOSE_API_CHOOSER(worldViewInvTranspose);

	perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetShadowMapTransform());
	perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix() * mParent->GetTransform()->GetWorldTransform());

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
		ConstantBuffer cbuffer2 = Renderer::Instance()->CreateConstantBuffer(2, sizeof(DeferredGeometryTerrainPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), effect->m_textureCB);
		DeferredGeometryTerrainPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (DeferredGeometryTerrainPassEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer2->GetBuffer();

		textureData->gShadowMap = LightManager::Instance()->m_shadowMap->GetDepthTexHandle();
		textureData->gHeightMap = reinterpret_cast<uint64_t>(m_heightMapTex->GetTexture());
		textureData->gLayerMapArray = reinterpret_cast<uint64_t>(m_layerMapArrayTex->GetTexture());
		textureData->gBlendMap = reinterpret_cast<uint64_t>(m_blendMapTex->GetTexture());

		cmd.cbuffers.push_back(std::move(cbuffer2));
		break;
	}
	}

	cmd.drawType = DrawType::INDEXED;
	cmd.flags = CmdFlag::DRAW | CmdFlag::BIND_FB;
	cmd.offset = (void*)(0);
	cmd.type = PrimitiveTopology::CONTROL_POINT_4_PATCHLIST;
	cmd.inputLayout = effect->GetInputLayout();
	cmd.framebuffer = Renderer::Instance()->GetGbuffer();
	cmd.vertexBuffer.push_back((void*)m_quadPatchVB->GetBuffer());
	cmd.indexBuffer = (void*)m_quadPatchIB->GetBuffer();
	cmd.vertexStride.push_back(stride);
	cmd.vertexOffset.push_back(offset);
	cmd.effect = effect;
	cmd.elementCount = m_numPatchQuadFaces * 4;
	cmd.cbuffers.push_back(std::move(cbuffer0));
	cmd.cbuffers.push_back(std::move(cbuffer1));

	Renderer::Instance()->AddDeferredDrawCmd(cmd);

	for (auto &grass : mGrasses)
	{
		grass.GatherDrawCall();
	}
}

void Terrain::GatherShadowDrawCall() {

	uint32_t stride = sizeof(Vertex::TerrainVertex);
	uint32_t offset = 0;

	TerrainShadowMapEffect* effect = EffectsManager::Instance()->m_terrainShadowMapEffect.get();

	ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(1, sizeof(TerrainShadowMapEffect::PERFRAME_CONSTANT_BUFFER), effect->m_perFrameCB);
	ConstantBuffer cbuffer1 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(TerrainShadowMapEffect::PEROBJ_CONSTANT_BUFFER), effect->m_perObjectCB);

	TerrainShadowMapEffect::PERFRAME_CONSTANT_BUFFER* perframeData = (TerrainShadowMapEffect::PERFRAME_CONSTANT_BUFFER*)cbuffer0->GetBuffer();
	TerrainShadowMapEffect::PEROBJ_CONSTANT_BUFFER* perObjData = (TerrainShadowMapEffect::PEROBJ_CONSTANT_BUFFER*)cbuffer1->GetBuffer();

	DrawCmd cmd;

	perframeData->gEyePosW = CameraManager::Instance()->GetActiveCamera()->GetPos();

	perframeData->gMaxDist = 500.00;
	perframeData->gMinDist = 20;
	perframeData->gMaxTess = 6.f;
	perframeData->gMinTess = 0.f;
	perframeData->gTexelCellSpaceU = 1.0f / m_initInfo.HeightmapWidth;
	perframeData->gTexelCellSpaceV = 1.0f / m_initInfo.HeightmapHeight;
	perframeData->gTexScale = Vec2(50.f, 50.f);
	perframeData->gWorldCellSpace = m_initInfo.CellSpacing;
	perframeData->gWorldFrustumPlanes /********************/;

	perObjData->View = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetViewMatrix());
	perObjData->ViewProj = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetViewProjMatrix());
	perObjData->World = TRASNPOSE_API_CHOOSER(mParent->GetTransform()->GetWorldTransform());
	perObjData->WorldInvTranspose = TRASNPOSE_API_CHOOSER(mParent->GetTransform()->GetWorldTransformInversed().Transpose());

	Vec4 resourceMask(0, 0, 0, 0);

	Mat4 worldView = LightManager::Instance()->m_shadowMap->GetViewMatrix() * mParent->GetTransform()->GetWorldTransform();
	perObjData->WorldView = TRASNPOSE_API_CHOOSER(worldView);

	Mat4 worldViewInvTranspose = worldView.Inverse().Transpose();
	perObjData->WorldViewInvTranspose = TRASNPOSE_API_CHOOSER(worldViewInvTranspose);

	//perObjData->ShadowTransform = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetShadowMapTransform());
	perObjData->WorldViewProj = TRASNPOSE_API_CHOOSER(LightManager::Instance()->m_shadowMap->GetViewProjMatrix() * mParent->GetTransform()->GetWorldTransform());

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
		ConstantBuffer cbuffer2 = Renderer::Instance()->CreateConstantBuffer(2, sizeof(TerrainShadowMapEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), effect->m_textureCB);
		TerrainShadowMapEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (TerrainShadowMapEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer2->GetBuffer();

		textureData->gShadowMap = LightManager::Instance()->m_shadowMap->GetDepthTex();
		textureData->gHeightMap = reinterpret_cast<uint64_t>(m_heightMapTex->GetTexture());
		textureData->gLayerMapArray = reinterpret_cast<uint64_t>(m_layerMapArrayTex->GetTexture());
		textureData->gBlendMap = reinterpret_cast<uint64_t>(m_blendMapTex->GetTexture());

		cmd.cbuffers.push_back(std::move(cbuffer2));

		break;
	}
	}

	cmd.drawType = DrawType::INDEXED;
	cmd.flags = CmdFlag::DRAW | CmdFlag::BIND_FB;
	cmd.offset = (void*)(0);
	cmd.type = PrimitiveTopology::CONTROL_POINT_4_PATCHLIST;
	cmd.inputLayout = effect->GetInputLayout();
	cmd.framebuffer = LightManager::Instance()->m_shadowMap->GetRenderTarget();
	cmd.vertexBuffer.push_back((void*)m_quadPatchVB->GetBuffer());
	cmd.indexBuffer = (void*)m_quadPatchIB->GetBuffer();
	cmd.vertexStride.push_back(stride);
	cmd.vertexOffset.push_back(offset);
	cmd.effect = effect;
	cmd.elementCount = m_numPatchQuadFaces * 4;
	cmd.cbuffers.push_back(std::move(cbuffer0));
	cmd.cbuffers.push_back(std::move(cbuffer1));

	Renderer::Instance()->AddDeferredDrawCmd(cmd);

}

const Mat4& Terrain::GetWorldTransform()
{
	return mParent->GetTransform()->GetWorldTransform();
}

void Terrain::DrawMenu()
{
	if (ImGui::CollapsingHeader("Terrain Renderer", ImGuiTreeNodeFlags_DefaultOpen))
	{

	}
}

}