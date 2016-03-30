#include "FbxReaderSG.h"
#include <fbxsdk.h>
#ifdef PLATFORM_WIN32
#include "ResourceManager.h"
#include "RendererBase.h"
#include "Shlwapi.h"
#include "SOIL.h"
#elif defined PLATFORM_ANDROID
#include "AndroidType.h"
#endif

#pragma warning(disable:4244) // conversion from 'fbxsdk_2015_1::FbxDouble' to 'float', possible loss of data

void ReadFbxMesh(FbxNode* node, Mesh* mesh);
void ReadFbxMaterial(FbxNode* node, Mesh* mesh);

bool FbxReaderSG::Read(const std::wstring& filename, Mesh* mesh) {

	std::string s(filename.begin(), filename.end());
	const char* lFilename = s.c_str();

	FbxManager* lSdkManager = FbxManager::Create();

	FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	// Create an importer using the SDK manager.
	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

	// Use the first argument as the filename for the importer.
	if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings())) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		return false;
	}

	FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");

	// Import the contents of the file into the scene.
	lImporter->Import(lScene);

	// The file is imported, so get rid of the importer.
	lImporter->Destroy();

	FbxNode* lRootNode = lScene->GetRootNode();

	if (lRootNode) {
		ReadFbxMesh(lRootNode->GetChild(0), mesh);
		ReadFbxMaterial(lRootNode->GetChild(0), mesh);
	}
	// Destroy the SDK manager and all the other objects it was handling.
	lSdkManager->Destroy();

	mesh->m_material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f);
	mesh->m_material.diffuse = XMFLOAT4(1.0f, 0.8f, 0.7f, 1.f);
	mesh->m_material.specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 10.0f);

	return true;
}

void ReadFbxMesh(FbxNode* node, Mesh* mesh) {
	FbxMesh* fbxMesh = node->GetMesh();

	//int layerCount = fbxMesh->getpol
	FbxLayerElementMaterial* mat = fbxMesh->GetLayer(0)->GetMaterials();
	FbxLayerElement::EMappingMode matMapping = mat->GetMappingMode();

	std::vector<UINT> polyMatIndex;

	if (matMapping == FbxLayerElement::eByPolygon) {
		int indexArrayCount = mat->GetIndexArray().GetCount();

		mesh->m_subMeshes.resize(node->GetMaterialCount());

		for (int i = 0; i < indexArrayCount; i++) {
			polyMatIndex.push_back(mat->GetIndexArray().GetAt(i));
		}
		int triangleCount = fbxMesh->GetPolygonCount();
		int counter = 0;
		for (int i = 0; i < triangleCount; i++){
			mesh->m_subMeshes[polyMatIndex[i]].m_indexBufferCPU.push_back(counter * 3);
			mesh->m_subMeshes[polyMatIndex[i]].m_indexBufferCPU.push_back(counter * 3 + 1);
			mesh->m_subMeshes[polyMatIndex[i]].m_indexBufferCPU.push_back(counter * 3 + 2);

			counter++;
		}
		for (uint32_t i = 0; i < mesh->m_subMeshes.size(); i++) {
			mesh->m_indexBufferCPU.insert(mesh->m_indexBufferCPU.end(), 
										  mesh->m_subMeshes[i].m_indexBufferCPU.begin(), 
										  mesh->m_subMeshes[i].m_indexBufferCPU.end());
		}
	}
	else if (matMapping == FbxLayerElement::eAllSame) {
		mesh->m_subMeshes.resize(1);
		int triangleCount = fbxMesh->GetPolygonCount();
		int counter = 0;
		for (int i = 0; i < triangleCount; i++){
			//fout << counter * 3 + 2 << " " << counter * 3 + 1 << " " << counter * 3 << std::endl;
			mesh->m_subMeshes[0].m_indexBufferCPU.push_back(counter * 3);
			mesh->m_subMeshes[0].m_indexBufferCPU.push_back(counter * 3 + 1);
			mesh->m_subMeshes[0].m_indexBufferCPU.push_back(counter * 3 + 2);
			counter++;
		}
		mesh->m_indexBufferCPU.insert(mesh->m_indexBufferCPU.end(),
			mesh->m_subMeshes[0].m_indexBufferCPU.begin(),
			mesh->m_subMeshes[0].m_indexBufferCPU.end());
	}
	FbxVector4* ControlPoints = fbxMesh->GetControlPoints();
	int PolygonCount = fbxMesh->GetPolygonCount();
	int numVerts = 0;
	for (int i = 0; i < PolygonCount; i++)
	{
		int PolygonSize = fbxMesh->GetPolygonSize(i);
		int vertexIndex = 0;
		for (int j = 0; j < PolygonSize; j++){
			int ControlPointIndex = fbxMesh->GetPolygonVertex(i, j);
			mesh->m_positionBufferCPU.push_back(XMFLOAT3((float)ControlPoints[ControlPointIndex][0], (float)ControlPoints[ControlPointIndex][1], (float)ControlPoints[ControlPointIndex][2]));
			numVerts++;

			int k = 0;
			bool flag = false;

			while (k < fbxMesh->GetElementUVCount() && !flag){
				FbxGeometryElementUV* UV = fbxMesh->GetElementUV(k);
				if (UV->GetMappingMode() == FbxGeometryElement::eByControlPoint){
					if (UV->GetReferenceMode() == FbxGeometryElement::eDirect){
						flag = true;
						mesh->m_texUVBufferCPU.push_back(XMFLOAT2(-UV->GetDirectArray().GetAt(ControlPointIndex).mData[0],
							-UV->GetDirectArray().GetAt(ControlPointIndex).mData[1]));
					}
					else if (UV->GetReferenceMode() == FbxGeometryElement::eIndexToDirect){
						flag = true;
						int id = UV->GetIndexArray().GetAt(ControlPointIndex);
						mesh->m_texUVBufferCPU.push_back(XMFLOAT2(-UV->GetDirectArray().GetAt(id).mData[0],
							-UV->GetDirectArray().GetAt(id).mData[1]));
					}
				}
				else if (UV->GetMappingMode() == FbxGeometryElement::eByPolygonVertex){
					if (UV->GetReferenceMode() == FbxGeometryElement::eDirect || UV->GetReferenceMode() == FbxGeometryElement::eIndexToDirect){
						flag = true;
						int TextureUVIndex = fbxMesh->GetTextureUVIndex(i, j);
						mesh->m_texUVBufferCPU.push_back(XMFLOAT2(-UV->GetDirectArray().GetAt(TextureUVIndex).mData[0],
							-UV->GetDirectArray().GetAt(TextureUVIndex).mData[1]));
					}
				}
				k++;
			}

			for (k = 0; k < fbxMesh->GetElementNormalCount(); k++){
				FbxGeometryElementNormal* Normal = fbxMesh->GetElementNormal(k);
				if (Normal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
					if (Normal->GetReferenceMode() == FbxGeometryElement::eDirect){
						mesh->m_normalBufferCPU.push_back(XMFLOAT3(Normal->GetDirectArray().GetAt(vertexIndex)[0], Normal->GetDirectArray().GetAt(vertexIndex)[1], -Normal->GetDirectArray().GetAt(vertexIndex)[2]));
					}
					else if (Normal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect){
						int id = Normal->GetIndexArray().GetAt(vertexIndex);
						mesh->m_normalBufferCPU.push_back(XMFLOAT3(Normal->GetDirectArray().GetAt(id)[0], Normal->GetDirectArray().GetAt(id)[1], -Normal->GetDirectArray().GetAt(id)[2]));
					}
				}
				else if (Normal->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
					if (Normal->GetReferenceMode() == FbxGeometryElement::eDirect){
						mesh->m_normalBufferCPU.push_back(XMFLOAT3(Normal->GetDirectArray().GetAt(ControlPointIndex).mData[0],
							Normal->GetDirectArray().GetAt(ControlPointIndex).mData[1],
							Normal->GetDirectArray().GetAt(ControlPointIndex).mData[2]));
					}
					else if (Normal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect){
						int id = Normal->GetIndexArray().GetAt(ControlPointIndex);
						mesh->m_normalBufferCPU.push_back(XMFLOAT3(Normal->GetDirectArray().GetAt(id)[0], Normal->GetDirectArray().GetAt(id)[1], -Normal->GetDirectArray().GetAt(id)[2]));
					}
				}
			}

			for (k = 0; k < fbxMesh->GetElementTangentCount(); k++){
				FbxGeometryElementTangent* Tangent = fbxMesh->GetElementTangent(k);
				if (Tangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
					if (Tangent->GetReferenceMode() == FbxGeometryElement::eDirect){
						mesh->m_tangentBufferCPU.push_back(XMFLOAT3(Tangent->GetDirectArray().GetAt(vertexIndex)[0], Tangent->GetDirectArray().GetAt(vertexIndex)[1], -Tangent->GetDirectArray().GetAt(vertexIndex)[2]));
					}
					else if (Tangent->GetReferenceMode() == FbxGeometryElement::eIndexToDirect){
						int id = Tangent->GetIndexArray().GetAt(vertexIndex);
						mesh->m_tangentBufferCPU.push_back(XMFLOAT3(Tangent->GetDirectArray().GetAt(id)[0], Tangent->GetDirectArray().GetAt(id)[1], -Tangent->GetDirectArray().GetAt(id)[2]));
					}
				}
				else if (Tangent->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
					if (Tangent->GetReferenceMode() == FbxGeometryElement::eDirect){
						mesh->m_tangentBufferCPU.push_back(XMFLOAT3(Tangent->GetDirectArray().GetAt(ControlPointIndex).mData[0],
							Tangent->GetDirectArray().GetAt(ControlPointIndex).mData[1],
							Tangent->GetDirectArray().GetAt(ControlPointIndex).mData[2]));
					}
					else if (Tangent->GetReferenceMode() == FbxGeometryElement::eIndexToDirect){
						int id = Tangent->GetIndexArray().GetAt(ControlPointIndex);
						mesh->m_tangentBufferCPU.push_back(XMFLOAT3(Tangent->GetDirectArray().GetAt(id)[0], Tangent->GetDirectArray().GetAt(id)[1], -Tangent->GetDirectArray().GetAt(id)[2]));
					}
				}
			}

			vertexIndex++;
		}
	}
}

void ReadFbxMaterial(FbxNode* node, Mesh* mesh) {
	//FbxMesh* fbxMesh = node->GetMesh();
	int matCount = node->GetMaterialCount();

	for (int i = 0; i < matCount; i++) {
		FbxSurfaceMaterial* sMat = node->GetMaterial(i);
		FbxProperty diffProp = sMat->FindProperty(FbxSurfaceMaterial::sDiffuse);
		/*
		int texLayerCount = diffProp.GetSrcObjectCount(FbxLayeredTexture::ClassId);
		if (texLayerCount > 0) {
			for (int iTexLayer = 0; iTexLayer < texLayerCount; iTexLayer++) {
				FbxObject* layeredTextures = diffProp.GetSrcObject(FbxLayeredTexture::ClassId, iTexLayer);
				int texCount = layeredTextures->GetSrcObjectCount(FbxTexture::ClassId);
				for (int iTex = 0; iTex < texCount; iTex++) {
					FbxObject* tex = layeredTextures->GetSrcObject(FbxTexture::ClassId, iTex);
					puts(tex->GetName());
					// TODO: unfinished!!
				}
			}
		}
		else {
		*/
			int texCount = diffProp.GetSrcObjectCount<FbxTexture>();
			for (int iTex = 0; iTex < texCount; iTex++) {
				FbxFileTexture* tex = FbxCast<FbxFileTexture>(diffProp.GetSrcObject<FbxTexture>(iTex));
				puts(tex->GetFileName());
				//if (matCount == 1)
				//	mesh->m_diffuseMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(tex->GetFileName());
				//else
#ifdef PLATFORM_WIN32
#ifdef GRAPHICS_D3D11
					mesh->m_subMeshes[i].m_diffuseMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(tex->GetFileName());
#else
					mesh->m_subMeshes[i].m_diffuseMapTex = *(ResourceManager::Instance()->GetResource<GLuint>(tex->GetFileName()));
#endif
#endif
			}
		//}

		FbxProperty normalProp = sMat->FindProperty(FbxSurfaceMaterial::sNormalMap);
		int normalTexCount = normalProp.GetSrcObjectCount<FbxTexture>();
		for (int iTex = 0; iTex < normalTexCount; iTex++) {
			FbxFileTexture* tex = FbxCast<FbxFileTexture>(normalProp.GetSrcObject<FbxTexture>(iTex));
			//if (matCount == 1)
			//	mesh->m_normalMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(tex->GetFileName());
			//else
#ifdef PLATFORM_WIN32
#ifdef GRAPHICS_D3D11
				mesh->m_subMeshes[i].m_normalMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(tex->GetFileName());
#else
				mesh->m_subMeshes[i].m_normalMapTex = *(ResourceManager::Instance()->GetResource<GLuint>(tex->GetFileName()));
#endif
#endif
		}

		FbxProperty displacementProp = sMat->FindProperty(FbxSurfaceMaterial::sDisplacementColor);
		int displacementTexCount = displacementProp.GetSrcObjectCount<FbxTexture>();
		for (int iTex = 0; iTex < displacementTexCount; iTex++) {
			FbxFileTexture* tex = FbxCast<FbxFileTexture>(displacementProp.GetSrcObject<FbxTexture>(iTex));
			//if (matCount == 1)
			//	mesh->m_bumpMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(tex->GetFileName());
			//else
#ifdef PLATFORM_WIN32
#ifdef GRAPHICS_D3D11
				mesh->m_subMeshes[i].m_bumpMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(tex->GetFileName());
				int a = 100;
#else
				mesh->m_subMeshes[i].m_bumpMapTex = *(ResourceManager::Instance()->GetResource<GLuint>(tex->GetFileName()));
#endif
#endif
		}
	}
}