#include "Utility/FbxReaderSG.h"
#include <algorithm>
#include <fbxsdk.h>
#include "Resource/ResourceManager.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Animation/Animation.h"
#include "Mesh/SkinnedMesh.h"
#include "Shlwapi.h"
#include <sys/stat.h>
#include <direct.h>

#pragma warning(disable:4244) // conversion from 'fbxsdk_2015_1::FbxDouble' to 'float', possible loss of data

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

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

	importer->ReadFile(lFilename, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_FixInfacingNormals | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);

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

		mesh->m_positionBufferCPU.reserve(fMesh->mNumVertices);
		mesh->m_texUVBufferCPU.reserve(fMesh->mNumVertices);
		mesh->m_normalBufferCPU.reserve(fMesh->mNumVertices);
		mesh->m_tangentBufferCPU.reserve(fMesh->mNumVertices);

		for (uint32_t j = 0; j < fMesh->mNumVertices; j++)
		{
			mesh->m_positionBufferCPU.push_back(XMFLOAT3(fMesh->mVertices[j].x, fMesh->mVertices[j].y, fMesh->mVertices[j].z));
			mesh->m_texUVBufferCPU.push_back(XMFLOAT2(fMesh->mTextureCoords[0][j].x, fMesh->mTextureCoords[0][j].y)); // load first uv set for now
			mesh->m_normalBufferCPU.push_back(XMFLOAT3(fMesh->mNormals[j].x, fMesh->mNormals[j].y, fMesh->mNormals[j].z));
			mesh->m_tangentBufferCPU.push_back(XMFLOAT3(fMesh->mTangents[j].x, fMesh->mTangents[j].y, fMesh->mTangents[j].z));
		}
		
		if (fMesh->HasBones())
		{
			std::unordered_map<std::string, uint32_t> mJointNameIndexMap;

			SkinnedMesh* skinnedMesh = dynamic_cast<SkinnedMesh*>(mesh);
			if (skinnedMesh)
			{
				skinnedMesh->m_jointWeightsBufferCPU.resize(fMesh->mNumVertices);
				skinnedMesh->m_jointIndicesBufferCPU.resize(fMesh->mNumVertices);

				skinnedMesh->m_jointOffsetTransformCPU.resize(fMesh->mNumBones);
				skinnedMesh->m_joints.resize(fMesh->mNumBones);

				for (uint32_t j = 0; j < fMesh->mNumBones; j++)
				{
					const auto &bone = fMesh->mBones[j];
					skinnedMesh->m_joints[j].m_index = j;
					skinnedMesh->m_joints[j].m_name = std::string(bone->mName.C_Str());
					mJointNameIndexMap.emplace(bone->mName.C_Str(), j);
					skinnedMesh->m_joints[j].m_inverseBindPosMat =
						DirectX::XMMATRIX(bone->mOffsetMatrix[0][0], bone->mOffsetMatrix[1][0], bone->mOffsetMatrix[2][0], bone->mOffsetMatrix[3][0], 
										  bone->mOffsetMatrix[0][1], bone->mOffsetMatrix[1][1], bone->mOffsetMatrix[2][1], bone->mOffsetMatrix[3][1], 
										  bone->mOffsetMatrix[0][2], bone->mOffsetMatrix[1][2], bone->mOffsetMatrix[2][2], bone->mOffsetMatrix[3][2], 
										  bone->mOffsetMatrix[0][3], bone->mOffsetMatrix[1][3], bone->mOffsetMatrix[2][3], bone->mOffsetMatrix[3][3]);
					for (uint32_t k = 0; k < bone->mNumWeights; k++)
					{
						auto weight = bone->mWeights[k];
						uint32_t vertexIdx = weight.mVertexId;
						skinnedMesh->m_jointWeightsBufferCPU[vertexIdx].push_back(weight.mWeight);
						skinnedMesh->m_jointIndicesBufferCPU[vertexIdx].push_back(j);
					}
				}
			}

			for (uint32_t j = 0; j < fMesh->mNumVertices; j++)
			{
				// Only support up to 4 joint weights
				skinnedMesh->m_jointWeightsBufferCPU[j].resize(4, 0.f); 
				skinnedMesh->m_jointIndicesBufferCPU[j].resize(4, 0.f);
			}

			auto assimp_node = scene->mRootNode;

			const XMMATRIX identityMat = XMMatrixIdentity();

			skinnedMesh->m_jointPreRotationBufferCPU.resize(fMesh->mNumBones, identityMat);
			std::function<void(aiNode*, int32_t)> IndexJoint;
			IndexJoint = [&](aiNode* node, int32_t parentIdx)
			{
				std::string nodeName(node->mName.C_Str());
				bool isPreRotation = false;
			   
				if (nodeName.find("_$AssimpFbx$_PreRotation") != std::string::npos)
				{
					nodeName.resize(nodeName.length() - strlen("_$AssimpFbx$_PreRotation"));
					isPreRotation = true;
				}

				auto entry = mJointNameIndexMap.find(nodeName);
				if (entry != mJointNameIndexMap.end())
				{
					if (isPreRotation)
					{
						skinnedMesh->m_jointPreRotationBufferCPU[entry->second] = 
							DirectX::XMMATRIX(node->mTransformation[0][0], node->mTransformation[1][0], node->mTransformation[2][0], node->mTransformation[3][0],
								node->mTransformation[0][1], node->mTransformation[1][1], node->mTransformation[2][1], node->mTransformation[3][1],
								node->mTransformation[0][2], node->mTransformation[1][2], node->mTransformation[2][2], node->mTransformation[3][2],
								node->mTransformation[0][3], node->mTransformation[1][3], node->mTransformation[2][3], node->mTransformation[3][3]);
					}
					else
					{
						skinnedMesh->m_joints[entry->second].m_parentIdx = parentIdx;
						parentIdx = entry->second;
					}
				}

				for (uint32_t ic = 0; ic < node->mNumChildren; ic++)
				{
					IndexJoint(node->mChildren[ic], parentIdx);
				}
			};

			IndexJoint(scene->mRootNode, -1);
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

bool FbxReaderSG::Read(const std::wstring& filename, Animation* animation) {

	std::string gFilename = std::string(filename.begin(), filename.end());

	const char* lFilename = gFilename.c_str();

	auto importer = new Assimp::Importer();

	importer->ReadFile(lFilename, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_FixInfacingNormals | aiProcess_OptimizeGraph);

	auto scene = importer->GetScene();

	for (uint32_t i = 0; i < scene->mNumAnimations; i++)
	{
		const auto &fAnimation = scene->mAnimations[i];

		for (uint32_t ichannel = 0; ichannel < fAnimation->mNumChannels; ichannel++)
		{
			const auto &channel = fAnimation->mChannels[ichannel];
			AnimationNode &animationNode = animation->m_animations[std::string(channel->mNodeName.C_Str())];

			animationNode.length = std::max(animationNode.length, channel->mNumPositionKeys);
			animationNode.position.resize(channel->mNumPositionKeys);
			animationNode.positionTime.resize(channel->mNumPositionKeys);
			for (uint32_t iPos = 0; iPos < channel->mNumPositionKeys; iPos++)
			{
				const auto &position = channel->mPositionKeys[iPos];
				XMFLOAT3 xmpos(position.mValue.x, position.mValue.y, position.mValue.z);
				animationNode.position[iPos] = XMLoadFloat3(&xmpos);
				animationNode.positionTime[iPos] = position.mTime;
			}

			animationNode.length = std::max(animationNode.length, channel->mNumRotationKeys);
			animationNode.rotation.resize(channel->mNumRotationKeys);
			animationNode.rotationTime.resize(channel->mNumRotationKeys);
			for (uint32_t iRot = 0; iRot < channel->mNumRotationKeys; iRot++)
			{
				const auto &rotation = channel->mRotationKeys[iRot];
				XMFLOAT4 xmrot(rotation.mValue.x, rotation.mValue.y, rotation.mValue.z, rotation.mValue.w);
				animationNode.rotation[iRot] = XMLoadFloat4(&xmrot);
				animationNode.rotationTime[iRot] = rotation.mTime;
			}

			animationNode.length = std::max(animationNode.length, channel->mNumScalingKeys);
			animationNode.scale.resize(channel->mNumScalingKeys);
			animationNode.scaleTime.resize(channel->mNumScalingKeys);
			for (uint32_t iScale = 0; iScale < channel->mNumScalingKeys; iScale++)
			{
				const auto &scale = channel->mScalingKeys[iScale];
				XMFLOAT3 xmscale(scale.mValue.x, scale.mValue.y, scale.mValue.z);
				animationNode.scale[iScale] = XMLoadFloat3(&xmscale);
				animationNode.scaleTime[iScale] = scale.mTime;
			}
		}
	}
	
	delete importer;

	return true;
}

}