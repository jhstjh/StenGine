#include "Terrain.h"
#include "RendererBase.h"
#include "D3DIncludes.h"
#include "CameraManager.h"
#include "LightManager.h"
#include "ResourceManager.h"
#include "ShadowMap.h"
#include <fstream>
#include "MathHelper.h"

Terrain* Terrain::_instance = nullptr;

Terrain::Terrain(InitInfo &info) :
m_quadPatchVB(0),
m_quadPatchIB(0),
m_layerMapArraySRV(0),
m_blendMapSRV(0),
m_heightMapSRV(0),
m_numPatchVertices(0),
m_numPatchQuadFaces(0),
m_numPatchVertRows(0),
m_numPatchVertCols(0),
m_initInfo(info)
{
	_instance = this;
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

	m_blendMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(m_initInfo.BlendMapFilename);
	m_layerMapArraySRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(m_initInfo.LayerMapFilenames);
}

Terrain::~Terrain() {

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

	if (inFile)	{
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

	std::vector<HALF> hmap(m_heightMap.size());
	std::transform(m_heightMap.begin(), m_heightMap.end(), hmap.begin(), XMConvertFloatToHalf);

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
	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateShaderResourceView(hmapTex, &srvDesc, &m_heightMapSRV));

	// SRV saves reference.
	ReleaseCOM(hmapTex);
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

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::TerrainVertex) * patchVertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &patchVertices[0];
	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&vbd, &vinitData, &m_quadPatchVB));

}

void Terrain::BuildQuadPatchIB() {
	std::vector<USHORT> indices(m_numPatchQuadFaces * 4);

	int k = 0;
	for (UINT i = 0; i < m_numPatchVertRows - 1; ++i) {
		for (UINT j = 0; j < m_numPatchVertCols - 1; ++j) {
			indices[k]		= i * m_numPatchVertCols + j;
			indices[k + 1]	= i * m_numPatchVertCols + j + 1;

			indices[k + 2]	= (i + 1) * m_numPatchVertCols + j;
			indices[k + 3]	= (i + 1) * m_numPatchVertCols + j + 1;

			k += 4;
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &indices[0];
	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&ibd, &initData, &m_quadPatchIB));
}

void Terrain::Draw() {
	UINT stride = sizeof(Vertex::TerrainVertex);
	UINT offset = 0;

	DeferredGeometryTerrainPassEffect* deferredGeoTerrainEffect = EffectsManager::Instance()->m_deferredGeometryTerrainPassEffect;
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->IASetInputLayout((ID3D11InputLayout *)deferredGeoTerrainEffect->GetInputLayout());
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->IASetVertexBuffers(0, 1, &m_quadPatchVB, &stride, &offset);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->IASetIndexBuffer(m_quadPatchIB, DXGI_FORMAT_R16_UINT, 0);
	
	deferredGeoTerrainEffect->SetShader();

	deferredGeoTerrainEffect->m_perFrameConstantBuffer.gEyePosW = CameraManager::Instance()->GetActiveCamera()->GetPos();
	deferredGeoTerrainEffect->m_perFrameConstantBuffer.gMaxDist = 500.00;
	deferredGeoTerrainEffect->m_perFrameConstantBuffer.gMinDist = 20;
	deferredGeoTerrainEffect->m_perFrameConstantBuffer.gMaxTess = 6.f;
	deferredGeoTerrainEffect->m_perFrameConstantBuffer.gMinTess = 0.f;
	deferredGeoTerrainEffect->m_perFrameConstantBuffer.gTexelCellSpaceU = 1.0f / m_initInfo.HeightmapWidth;
	deferredGeoTerrainEffect->m_perFrameConstantBuffer.gTexelCellSpaceV = 1.0f / m_initInfo.HeightmapHeight;
	deferredGeoTerrainEffect->m_perFrameConstantBuffer.gTexScale = XMFLOAT2(50.f, 50.f);
	deferredGeoTerrainEffect->m_perFrameConstantBuffer.gWorldCellSpace = m_initInfo.CellSpacing;
	deferredGeoTerrainEffect->m_perFrameConstantBuffer.gWorldFrustumPlanes /********************/;
	
	deferredGeoTerrainEffect->m_perObjConstantBuffer.View = XMMatrixTranspose(CameraManager::Instance()->GetActiveCamera()->GetViewMatrix());
	deferredGeoTerrainEffect->m_perObjConstantBuffer.ViewProj = XMMatrixTranspose(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());
	deferredGeoTerrainEffect->m_perObjConstantBuffer.World = XMMatrixTranspose(XMLoadFloat4x4(m_parents[0]->GetWorldTransform()));
	deferredGeoTerrainEffect->m_perObjConstantBuffer.WorldInvTranspose = XMMatrixTranspose(MatrixHelper::InverseTranspose(XMLoadFloat4x4(m_parents[0]->GetWorldTransform())));

	XMMATRIX worldView = XMLoadFloat4x4(m_parents[0]->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewMatrix();
	deferredGeoTerrainEffect->m_perObjConstantBuffer.WorldView = XMMatrixTranspose(worldView);

	XMMATRIX worldViewInvTranspose = MatrixHelper::InverseTranspose(worldView);
	deferredGeoTerrainEffect->m_perObjConstantBuffer.WorldViewInvTranspose = XMMatrixTranspose(worldViewInvTranspose);

	deferredGeoTerrainEffect->m_perObjConstantBuffer.ShadowTransform = XMMatrixTranspose(LightManager::Instance()->m_shadowMap->GetShadowMapTransform());
	deferredGeoTerrainEffect->m_perObjConstantBuffer.WorldViewProj = XMMatrixTranspose(XMLoadFloat4x4(m_parents[0]->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

	deferredGeoTerrainEffect->UpdateConstantBuffer();
	deferredGeoTerrainEffect->BindConstantBuffer();

	deferredGeoTerrainEffect->SetShaderResources(LightManager::Instance()->m_shadowMap->GetDepthSRV(), 3);
	deferredGeoTerrainEffect->SetShaderResources(m_heightMapSRV, 5);
	deferredGeoTerrainEffect->SetShaderResources(m_layerMapArraySRV, 6);
	deferredGeoTerrainEffect->SetShaderResources(m_blendMapSRV, 7);
	deferredGeoTerrainEffect->BindShaderResource();

	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DrawIndexed(m_numPatchQuadFaces * 4, 0, 0);

	deferredGeoTerrainEffect->UnBindConstantBuffer();
	deferredGeoTerrainEffect->UnBindShaderResource();
	deferredGeoTerrainEffect->UnSetShader();
}

void Terrain::DrawOnShadowMap() {
	UINT stride = sizeof(Vertex::TerrainVertex);
	UINT offset = 0;

	TerrainShadowMapEffect* terrainShadowMapEffect = EffectsManager::Instance()->m_terrainShadowMapEffect;
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->IASetInputLayout((ID3D11InputLayout *)terrainShadowMapEffect->GetInputLayout());
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->IASetVertexBuffers(0, 1, &m_quadPatchVB, &stride, &offset);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->IASetIndexBuffer(m_quadPatchIB, DXGI_FORMAT_R16_UINT, 0);

	terrainShadowMapEffect->SetShader();

	terrainShadowMapEffect->m_perFrameConstantBuffer.gEyePosW = CameraManager::Instance()->GetActiveCamera()->GetPos();
	terrainShadowMapEffect->m_perFrameConstantBuffer.gMaxDist = 500.00;
	terrainShadowMapEffect->m_perFrameConstantBuffer.gMinDist = 20;
	terrainShadowMapEffect->m_perFrameConstantBuffer.gMaxTess = 6.f;
	terrainShadowMapEffect->m_perFrameConstantBuffer.gMinTess = 0.f;
	terrainShadowMapEffect->m_perFrameConstantBuffer.gTexelCellSpaceU = 1.0f / m_initInfo.HeightmapWidth;
	terrainShadowMapEffect->m_perFrameConstantBuffer.gTexelCellSpaceV = 1.0f / m_initInfo.HeightmapHeight;
	terrainShadowMapEffect->m_perFrameConstantBuffer.gTexScale = XMFLOAT2(50.f, 50.f);
	terrainShadowMapEffect->m_perFrameConstantBuffer.gWorldCellSpace = m_initInfo.CellSpacing;
	terrainShadowMapEffect->m_perFrameConstantBuffer.gWorldFrustumPlanes /********************/;

	terrainShadowMapEffect->m_perObjConstantBuffer.View = XMMatrixTranspose(CameraManager::Instance()->GetActiveCamera()->GetViewMatrix());
	terrainShadowMapEffect->m_perObjConstantBuffer.ViewProj = XMMatrixTranspose(CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());
	terrainShadowMapEffect->m_perObjConstantBuffer.World = XMMatrixTranspose(XMLoadFloat4x4(m_parents[0]->GetWorldTransform()));
	terrainShadowMapEffect->m_perObjConstantBuffer.WorldInvTranspose = XMMatrixTranspose(MatrixHelper::InverseTranspose(XMLoadFloat4x4(m_parents[0]->GetWorldTransform())));

	XMMATRIX worldView = XMLoadFloat4x4(m_parents[0]->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewMatrix();
	terrainShadowMapEffect->m_perObjConstantBuffer.WorldView = XMMatrixTranspose(worldView);

	XMMATRIX worldViewInvTranspose = MatrixHelper::InverseTranspose(worldView);
	terrainShadowMapEffect->m_perObjConstantBuffer.WorldViewInvTranspose = XMMatrixTranspose(worldViewInvTranspose);

	//terrainShadowMapEffect->m_perObjConstantBuffer.ShadowTransform = XMMatrixTranspose(XMLoadFloat4x4(m_parents[0]->GetWorldTransform()) * LightManager::Instance()->m_shadowMap->GetShadowMapTransform());
	terrainShadowMapEffect->m_perObjConstantBuffer.WorldViewProj = XMMatrixTranspose(XMLoadFloat4x4(m_parents[0]->GetWorldTransform()) * CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix());

	terrainShadowMapEffect->UpdateConstantBuffer();
	terrainShadowMapEffect->BindConstantBuffer();

	terrainShadowMapEffect->SetShaderResources(m_heightMapSRV, 5);
	terrainShadowMapEffect->SetShaderResources(m_layerMapArraySRV, 6);
	terrainShadowMapEffect->SetShaderResources(m_blendMapSRV, 7);
	terrainShadowMapEffect->BindShaderResource();

	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DrawIndexed(m_numPatchQuadFaces * 4, 0, 0);

	terrainShadowMapEffect->UnBindConstantBuffer();
	terrainShadowMapEffect->UnBindShaderResource();
	terrainShadowMapEffect->UnSetShader();
}