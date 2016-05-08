#include "Utility/FbxReaderSG.h"
#include <fbxsdk.h>
#include "Resource/ResourceManager.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Shlwapi.h"
#include <sys/stat.h>
#include <direct.h>

#pragma warning(disable:4244) // conversion from 'fbxsdk_2015_1::FbxDouble' to 'float', possible loss of data

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#define USE_MODEL_CACHE 0

namespace StenGine
{

void ReadFbxMesh(FbxNode* node, Mesh* mesh, const std::wstring& filename);
void ReadFbxMaterial(FbxNode* node, Mesh* mesh, const std::wstring& filename);
bool ReadModelCache(Mesh* mesh, const std::wstring& filename);

bool FbxReaderSG::Read(const std::wstring& filename, Mesh* mesh) {

	std::string gFilename = std::string(filename.begin(), filename.end());

#if USE_MODEL_CACHE
	if (ReadModelCache(mesh, filename))
		return true;
#endif

	const char* lFilename = gFilename.c_str();
#if 0
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
		ReadFbxMesh(lRootNode->GetChild(0), mesh, filename);
		ReadFbxMaterial(lRootNode->GetChild(0), mesh, filename);
	}
	// Destroy the SDK manager and all the other objects it was handling.
	lSdkManager->Destroy();

	return true;
#endif

	auto importer = new Assimp::Importer();

	importer->ReadFile(lFilename, 0);

	auto scene = importer->GetScene();

	mesh->m_subMeshes.resize(scene->mNumMeshes);

	uint32_t counter = 0;
	for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
	{
		auto &fMesh = scene->mMeshes[i];
		uint32_t triangleCount = fMesh->mNumFaces; // assume it is a triangle

		for (uint32_t t = 0; t < triangleCount; ++t)
		{
			mesh->m_subMeshes[i].m_indexBufferCPU.push_back(counter * 3);
			mesh->m_subMeshes[i].m_indexBufferCPU.push_back(counter * 3 + 1);
			mesh->m_subMeshes[i].m_indexBufferCPU.push_back(counter * 3 + 2);
			counter++;
		}
	}
	for (uint32_t i = 0; i < mesh->m_subMeshes.size(); i++) {
		mesh->m_indexBufferCPU.insert(mesh->m_indexBufferCPU.end(),
			mesh->m_subMeshes[i].m_indexBufferCPU.begin(),
			mesh->m_subMeshes[i].m_indexBufferCPU.end());
	}

	for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
	{
		auto &fMesh = scene->mMeshes[i];
		uint32_t triangleCount = fMesh->mNumFaces; // assume it is a triangle

		mesh->m_subMeshes[i].m_matIndex = fMesh->mMaterialIndex;

		for (uint32_t j = 0; j < fMesh->mNumVertices; j++)
		{
			mesh->m_positionBufferCPU.push_back(XMFLOAT3(fMesh->mVertices[j].x, fMesh->mVertices[j].y, fMesh->mVertices[j].z));
			mesh->m_texUVBufferCPU.push_back(XMFLOAT2(fMesh->mTextureCoords[0][j].x, fMesh->mTextureCoords[0][j].y)); // load first uv set for now
			mesh->m_normalBufferCPU.push_back(XMFLOAT3(fMesh->mNormals[j].x, fMesh->mNormals[j].y, fMesh->mNormals[j].z));
			mesh->m_tangentBufferCPU.push_back(XMFLOAT3(fMesh->mTangents[j].x, fMesh->mTangents[j].y, fMesh->mTangents[j].z));
		}
	}

	mesh->m_materials.resize(scene->mNumMaterials);
	for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
	{
		
		auto &fMat = scene->mMaterials[i];

		char dir[512];
		_getcwd(dir, 512);

		std::string dirStr(dir);

		aiString texPath;
		if (fMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			mesh->m_materials[i].m_diffuseMapTex = ResourceManager::Instance()->GetResource<Texture>((dirStr + "\\Model\\" + texPath.C_Str()).c_str());
		}

		if (fMat->GetTexture(aiTextureType_NORMALS, 0, &texPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			mesh->m_materials[i].m_normalMapTex = ResourceManager::Instance()->GetResource<Texture>((dirStr + "\\Model\\" + texPath.C_Str()).c_str());
		}

		if (fMat->GetTexture(aiTextureType_DISPLACEMENT, 0, &texPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			mesh->m_materials[i].m_bumpMapTex = ResourceManager::Instance()->GetResource<Texture>((dirStr + "\\Model\\" + texPath.C_Str()).c_str());
		}

	}
	delete importer;

	return true;
}

#if 0

void ReadFbxMesh(FbxNode* node, Mesh* mesh, const std::wstring& filename) {
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
		for (int i = 0; i < triangleCount; i++) {
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
		for (int i = 0; i < triangleCount; i++) {
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
		for (int j = 0; j < PolygonSize; j++) {
			int ControlPointIndex = fbxMesh->GetPolygonVertex(i, j);
			mesh->m_positionBufferCPU.push_back(XMFLOAT3((float)ControlPoints[ControlPointIndex][0], (float)ControlPoints[ControlPointIndex][1], (float)ControlPoints[ControlPointIndex][2]));
			numVerts++;

			int k = 0;
			bool flag = false;

			while (k < fbxMesh->GetElementUVCount() && !flag) {
				FbxGeometryElementUV* UV = fbxMesh->GetElementUV(k);
				if (UV->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
					if (UV->GetReferenceMode() == FbxGeometryElement::eDirect) {
						flag = true;
						mesh->m_texUVBufferCPU.push_back(XMFLOAT2(-UV->GetDirectArray().GetAt(ControlPointIndex).mData[0],
							-UV->GetDirectArray().GetAt(ControlPointIndex).mData[1]));
					}
					else if (UV->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
						flag = true;
						int id = UV->GetIndexArray().GetAt(ControlPointIndex);
						mesh->m_texUVBufferCPU.push_back(XMFLOAT2(-UV->GetDirectArray().GetAt(id).mData[0],
							-UV->GetDirectArray().GetAt(id).mData[1]));
					}
				}
				else if (UV->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
					if (UV->GetReferenceMode() == FbxGeometryElement::eDirect || UV->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
						flag = true;
						int TextureUVIndex = fbxMesh->GetTextureUVIndex(i, j);
						mesh->m_texUVBufferCPU.push_back(XMFLOAT2(-UV->GetDirectArray().GetAt(TextureUVIndex).mData[0],
							-UV->GetDirectArray().GetAt(TextureUVIndex).mData[1]));
					}
				}
				k++;
			}

			for (k = 0; k < fbxMesh->GetElementNormalCount(); k++) {
				FbxGeometryElementNormal* Normal = fbxMesh->GetElementNormal(k);
				if (Normal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
					if (Normal->GetReferenceMode() == FbxGeometryElement::eDirect) {
						mesh->m_normalBufferCPU.push_back(XMFLOAT3(Normal->GetDirectArray().GetAt(vertexIndex)[0], Normal->GetDirectArray().GetAt(vertexIndex)[1], -Normal->GetDirectArray().GetAt(vertexIndex)[2]));
					}
					else if (Normal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
						int id = Normal->GetIndexArray().GetAt(vertexIndex);
						mesh->m_normalBufferCPU.push_back(XMFLOAT3(Normal->GetDirectArray().GetAt(id)[0], Normal->GetDirectArray().GetAt(id)[1], -Normal->GetDirectArray().GetAt(id)[2]));
					}
				}
				else if (Normal->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
					if (Normal->GetReferenceMode() == FbxGeometryElement::eDirect) {
						mesh->m_normalBufferCPU.push_back(XMFLOAT3(Normal->GetDirectArray().GetAt(ControlPointIndex).mData[0],
							Normal->GetDirectArray().GetAt(ControlPointIndex).mData[1],
							Normal->GetDirectArray().GetAt(ControlPointIndex).mData[2]));
					}
					else if (Normal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
						int id = Normal->GetIndexArray().GetAt(ControlPointIndex);
						mesh->m_normalBufferCPU.push_back(XMFLOAT3(Normal->GetDirectArray().GetAt(id)[0], Normal->GetDirectArray().GetAt(id)[1], -Normal->GetDirectArray().GetAt(id)[2]));
					}
				}
			}

			for (k = 0; k < fbxMesh->GetElementTangentCount(); k++) {
				FbxGeometryElementTangent* Tangent = fbxMesh->GetElementTangent(k);
				if (Tangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
					if (Tangent->GetReferenceMode() == FbxGeometryElement::eDirect) {
						mesh->m_tangentBufferCPU.push_back(XMFLOAT3(Tangent->GetDirectArray().GetAt(vertexIndex)[0], Tangent->GetDirectArray().GetAt(vertexIndex)[1], -Tangent->GetDirectArray().GetAt(vertexIndex)[2]));
					}
					else if (Tangent->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
						int id = Tangent->GetIndexArray().GetAt(vertexIndex);
						mesh->m_tangentBufferCPU.push_back(XMFLOAT3(Tangent->GetDirectArray().GetAt(id)[0], Tangent->GetDirectArray().GetAt(id)[1], -Tangent->GetDirectArray().GetAt(id)[2]));
					}
				}
				else if (Tangent->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
					if (Tangent->GetReferenceMode() == FbxGeometryElement::eDirect) {
						mesh->m_tangentBufferCPU.push_back(XMFLOAT3(Tangent->GetDirectArray().GetAt(ControlPointIndex).mData[0],
							Tangent->GetDirectArray().GetAt(ControlPointIndex).mData[1],
							Tangent->GetDirectArray().GetAt(ControlPointIndex).mData[2]));
					}
					else if (Tangent->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
						int id = Tangent->GetIndexArray().GetAt(ControlPointIndex);
						mesh->m_tangentBufferCPU.push_back(XMFLOAT3(Tangent->GetDirectArray().GetAt(id)[0], Tangent->GetDirectArray().GetAt(id)[1], -Tangent->GetDirectArray().GetAt(id)[2]));
					}
				}
			}

			vertexIndex++;
		}
	}

#if USE_MODEL_CACHE
	std::fstream fs;
	fs.open(filename + L".sgm", std::fstream::out | std::fstream::binary);

	// dump position buffer
	fs << mesh->m_positionBufferCPU.size() << std::endl;
	fs << mesh->m_positionBufferCPU.size() * sizeof(XMFLOAT3) << std::endl;
	fs.write((const char*)mesh->m_positionBufferCPU.data(), mesh->m_positionBufferCPU.size() * sizeof(XMFLOAT3));
	fs << std::endl;
	//for (int i = 0; i < mesh->m_positionBufferCPU.size(); i++) {
	//	fs << mesh->m_positionBufferCPU[i].x << " "
	//		<< mesh->m_positionBufferCPU[i].y << " "
	//		<< mesh->m_positionBufferCPU[i].z << std::endl;
	//}

	// dump normal buffer
	fs << mesh->m_normalBufferCPU.size() << std::endl;
	fs << mesh->m_normalBufferCPU.size() * sizeof(XMFLOAT3) << std::endl;
	fs.write((const char*)mesh->m_normalBufferCPU.data(), mesh->m_normalBufferCPU.size() * sizeof(XMFLOAT3));
	fs << std::endl;
	//for (int i = 0; i < mesh->m_normalBufferCPU.size(); i++) {
	//	fs << mesh->m_normalBufferCPU[i].x << " "
	//		<< mesh->m_normalBufferCPU[i].y << " "
	//		<< mesh->m_normalBufferCPU[i].z << std::endl;
	//}

	// dump tangent buffer
	fs << mesh->m_tangentBufferCPU.size() << std::endl;
	fs << mesh->m_tangentBufferCPU.size() * sizeof(XMFLOAT3) << std::endl;
	fs.write((const char*)mesh->m_tangentBufferCPU.data(), mesh->m_tangentBufferCPU.size() * sizeof(XMFLOAT3));
	fs << std::endl;
	//for (int i = 0; i < mesh->m_tangentBufferCPU.size(); i++) {
	//	fs << mesh->m_tangentBufferCPU[i].x << " "
	//		<< mesh->m_tangentBufferCPU[i].y << " "
	//		<< mesh->m_tangentBufferCPU[i].z << std::endl;
	//}

	// dump uv buffer
	fs << mesh->m_texUVBufferCPU.size() << std::endl;
	fs << mesh->m_texUVBufferCPU.size() * sizeof(XMFLOAT2) << std::endl;
	fs.write((const char*)mesh->m_texUVBufferCPU.data(), mesh->m_texUVBufferCPU.size() * sizeof(XMFLOAT2));
	fs << std::endl;
	//for (int i = 0; i < mesh->m_texUVBufferCPU.size(); i++) {
	//	fs << mesh->m_texUVBufferCPU[i].x << " "
	//		<< mesh->m_texUVBufferCPU[i].y << std::endl;
	//}

	// dump index buffer for each submesh
	fs << mesh->m_subMeshes.size() << std::endl;
	for (int i = 0; i < mesh->m_subMeshes.size(); i++) {
		fs << mesh->m_subMeshes[i].m_indexBufferCPU.size() << std::endl;
		for (int j = 0; j < mesh->m_subMeshes[i].m_indexBufferCPU.size(); j++) {
			fs << mesh->m_subMeshes[i].m_indexBufferCPU[j] << " ";
		}
		fs << std::endl;
	}
	fs << std::endl;

	fs.close();
#endif
}

void ReadFbxMaterial(FbxNode* node, Mesh* mesh, const std::wstring& filename) {
	//FbxMesh* fbxMesh = node->GetMesh();
	int matCount = node->GetMaterialCount();

#if USE_MODEL_CACHE
	std::fstream fs;
	fs.open(filename + L".sgm", std::fstream::app | std::fstream::binary);

	fs << matCount;
#endif

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
		bool has = false;
		int texCount = diffProp.GetSrcObjectCount<FbxTexture>();
		for (int iTex = 0; iTex < texCount; iTex++) {
			FbxFileTexture* tex = FbxCast<FbxFileTexture>(diffProp.GetSrcObject<FbxTexture>(iTex));
			puts(tex->GetFileName());
#if GRAPHICS_D3D11
			mesh->m_subMeshes[i].m_diffuseMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(tex->GetFileName());
#else
			mesh->m_subMeshes[i].m_diffuseMapTex = *(ResourceManager::Instance()->GetResource<uint64_t>(tex->GetFileName()));
#endif
			has = true;
#if USE_MODEL_CACHE
			fs << std::endl << 1;
			fs << std::endl << tex->GetFileName();
#endif
		}
		//}
#if USE_MODEL_CACHE
		if (!has)
			fs << std::endl << 0;
#endif

		has = false;
		FbxProperty normalProp = sMat->FindProperty(FbxSurfaceMaterial::sNormalMap);
		int normalTexCount = normalProp.GetSrcObjectCount<FbxTexture>();
		for (int iTex = 0; iTex < normalTexCount; iTex++) {
			FbxFileTexture* tex = FbxCast<FbxFileTexture>(normalProp.GetSrcObject<FbxTexture>(iTex));
#if GRAPHICS_D3D11
			mesh->m_subMeshes[i].m_normalMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(tex->GetFileName());
#else
			mesh->m_subMeshes[i].m_normalMapTex = *(ResourceManager::Instance()->GetResource<uint64_t>(tex->GetFileName()));
#endif
			has = true;
#if USE_MODEL_CACHE
			fs << std::endl << 1;
			fs << std::endl << tex->GetFileName();
#endif
		}

#if USE_MODEL_CACHE
		if (!has)
			fs << std::endl << 0;
#endif

		has = false;
		FbxProperty displacementProp = sMat->FindProperty(FbxSurfaceMaterial::sDisplacementColor);
		int displacementTexCount = displacementProp.GetSrcObjectCount<FbxTexture>();
		for (int iTex = 0; iTex < displacementTexCount; iTex++) {
			FbxFileTexture* tex = FbxCast<FbxFileTexture>(displacementProp.GetSrcObject<FbxTexture>(iTex));
#if GRAPHICS_D3D11
			mesh->m_subMeshes[i].m_bumpMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(tex->GetFileName());
#else
			mesh->m_subMeshes[i].m_bumpMapTex = *(ResourceManager::Instance()->GetResource<uint64_t>(tex->GetFileName()));
#endif
			has = true;
#if USE_MODEL_CACHE
			fs << std::endl << 1;
			fs << std::endl << tex->GetFileName();
#endif
		}
#if USE_MODEL_CACHE
		if (!has)
			fs << std::endl << 0;
#endif
	}

#if USE_MODEL_CACHE
	fs.close();
#endif
}

bool ReadModelCache(Mesh* mesh, const std::wstring& filename)
{
	std::string f(filename.begin(), filename.end());

	struct stat buf;
	if (stat((f + ".sgm").c_str(), &buf) != 0)
		return false;

	printf("Reading model cache for: %s\r\n", f.c_str());

	std::fstream fs;
	fs.open(filename + L".sgm", std::fstream::in | std::fstream::binary);

	int posBufferSize;
	int memsize;
	fs >> posBufferSize;
	fs >> memsize;
	mesh->m_positionBufferCPU.resize(posBufferSize);
	fs.get();
	fs.read((char* )&mesh->m_positionBufferCPU[0], memsize);
	//for (int i = 0; i < posBufferSize; i++) {
	//	float x, y, z;
	//	fs >> x >> y >> z;
	//
	//	mesh->m_positionBufferCPU[i] = XMFLOAT3(x, y, z);
	//}
	fs.get();

	int normalBufferSize;
	fs >> normalBufferSize;
	fs >> memsize;
	mesh->m_normalBufferCPU.resize(normalBufferSize);
	fs.get();
	fs.read((char*)&mesh->m_normalBufferCPU[0], memsize);
	//for (int i = 0; i < normalBufferSize; i++) {
	//	float x, y, z;
	//	fs >> x >> y >> z;
	//
	//	mesh->m_normalBufferCPU[i] = XMFLOAT3(x, y, z);
	//}
	fs.get();

	int tangentBufferSize;
	fs >> tangentBufferSize;
	fs >> memsize;
	mesh->m_tangentBufferCPU.resize(tangentBufferSize);
	fs.get();
	fs.read((char*)&mesh->m_tangentBufferCPU[0], memsize);
	//for (int i = 0; i < tangentBufferSize; i++) {
	//	float x, y, z;
	//	fs >> x >> y >> z;
	//
	//	mesh->m_tangentBufferCPU[i] = XMFLOAT3(x, y, z);
	//}
	fs.get();

	int texUVBufferSize;
	fs >> texUVBufferSize;
	fs >> memsize;
	mesh->m_texUVBufferCPU.resize(texUVBufferSize);
	fs.get();
	fs.read((char*)&mesh->m_texUVBufferCPU[0], memsize);
	//for (int i = 0; i < texUVBufferSize; i++) {
	//	float x, y;
	//	fs >> x >> y;
	//
	//	mesh->m_texUVBufferCPU[i] = XMFLOAT2(x, y);
	//}
	fs.get();

	int subMeshCount;
	fs >> subMeshCount;
	mesh->m_subMeshes.resize(subMeshCount);
	for (int i = 0; i < subMeshCount; i++) {
		int indexBufferSize;
		fs >> indexBufferSize;
		mesh->m_subMeshes[i].m_indexBufferCPU.resize(indexBufferSize);
		for (int j = 0; j < indexBufferSize; j++) {
			fs >> mesh->m_subMeshes[i].m_indexBufferCPU[j];
		}
	}

	for (int i = 0; i < subMeshCount; i++) {
		mesh->m_indexBufferCPU.insert(mesh->m_indexBufferCPU.end(), mesh->m_subMeshes[i].m_indexBufferCPU.begin(), mesh->m_subMeshes[i].m_indexBufferCPU.end());
	}

	int32_t matCount;
	fs >> matCount;

	if (fs.eof())
		return true;

	for (int32_t i = 0; i < matCount; i++)
	{
		int32_t exist;
		fs >> exist;

		if (exist == 1)
		{
			char diffTexName[256];
			fs.get(); // lineending
			fs.getline(diffTexName, 256);
			
#if GRAPHICS_D3D11
			mesh->m_subMeshes[i].m_diffuseMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(diffTexName);
#else
			mesh->m_subMeshes[i].m_diffuseMapTex = *(ResourceManager::Instance()->GetResource<uint64_t>(diffTexName));
#endif
			if (fs.eof())
				break;
		}

		fs >> exist;
		if (exist == 1)
		{
			char normalTexName[256];
			fs.get(); // lineending
			fs.getline(normalTexName, 256);
			
#if GRAPHICS_D3D11
			mesh->m_subMeshes[i].m_normalMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(normalTexName);
#else
			mesh->m_subMeshes[i].m_normalMapTex = *(ResourceManager::Instance()->GetResource<uint64_t>(normalTexName));
#endif
			if (fs.eof())
				break;
		}

		fs >> exist;
		if (exist == 1)
		{
			char bumpTexName[256];
			fs.get(); // lineending
			fs.getline(bumpTexName, 256);
			
#if GRAPHICS_D3D11
			mesh->m_subMeshes[i].m_bumpMapSRV = ResourceManager::Instance()->GetResource<ID3D11ShaderResourceView>(bumpTexName);
#else
			mesh->m_subMeshes[i].m_bumpMapTex = *(ResourceManager::Instance()->GetResource<uint64_t>(bumpTexName));
#endif
			if (fs.eof())
				break;
		}

	}

	return true;
}
#endif

}