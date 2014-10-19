#include "FbxReaderSG.h"
#include <fbxsdk.h>
#include "D3D11Renderer.h"

void ReadFbxMesh(FbxNode* node, Mesh* mesh);

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
// 		for (int i = 0; i < lRootNode->GetChildCount(); i++) {
// 			//PrintNode(lRootNode->GetChild(i));
// 		}
		ReadFbxMesh(lRootNode->GetChild(0), mesh);
	}
	// Destroy the SDK manager and all the other objects it was handling.
	lSdkManager->Destroy();

	return true;
}

void ReadFbxMesh(FbxNode* node, Mesh* mesh) {
	FbxMesh* fbxMesh = node->GetMesh();

	int triangleCount = fbxMesh->GetPolygonCount();
	int counter = 0;
	for (int i = 0; i < triangleCount; i++){
		//fout << counter * 3 + 2 << " " << counter * 3 + 1 << " " << counter * 3 << std::endl;
		mesh->m_indexBufferCPU.push_back(counter * 3);
		mesh->m_indexBufferCPU.push_back(counter * 3 + 1);
		mesh->m_indexBufferCPU.push_back(counter * 3 + 2);
		counter++;
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
			vertexIndex++;
		}


	}
	//mesh->m_texUVBufferCPU.resize(mesh->m_positionBufferCPU.size());

	mesh->m_material.ambient = XMFLOAT4(0.2, 0.2, 0.2, 1);
	mesh->m_material.diffuse = XMFLOAT4(1.0, 0.5, 0.3, 1);
	mesh->m_material.specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 10.0f);


	HR(D3DX11CreateShaderResourceViewFromFile(
		D3D11Renderer::Instance()->GetD3DDevice(),
		L"Model/earth.dds", 0, 0, &(mesh->m_diffuseMapSRV), 0));
}