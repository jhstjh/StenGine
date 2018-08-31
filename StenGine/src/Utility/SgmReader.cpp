#include "stdafx.h"

#if 0

#include "Utility/SgmReader.h"
#include <vector>
#include <sstream>
#include <string>

namespace StenGine
{

void ReadSgmMesh(std::vector<uint8_t> &data, Mesh* mesh) {

}

bool SgmReader::Read(const std::string& filename, Mesh* mesh) {
	std::vector<uint8_t> data;
	bool b = JNIHelper::GetInstance()->ReadFile(filename.c_str(), &data);

	if (b) {
		std::string cData;

		for (int i = 0; i < data.size(); i++) {
			cData += data[i];
		}

		std::istringstream sData(cData);

		int posBufferSize;
		sData >> posBufferSize;
		mesh->m_positionBufferCPU.resize(posBufferSize);
		for (int i = 0; i < posBufferSize; i++) {
			float x, y, z;
			sData >> x >> y >> z;
				
			mesh->m_positionBufferCPU[i] = XMFLOAT3(x, y, z);
		}

		int normalBufferSize;
		sData >> normalBufferSize;
		mesh->m_normalBufferCPU.resize(normalBufferSize);
		for (int i = 0; i < normalBufferSize; i++) {
			float x, y, z;
			sData >> x >> y >> z;

			mesh->m_normalBufferCPU[i] = XMFLOAT3(x, y, z);
		}

		int tangentBufferSize;
		sData >> tangentBufferSize;
		mesh->m_tangentBufferCPU.resize(tangentBufferSize);
		for (int i = 0; i < tangentBufferSize; i++) {
			float x, y, z;
			sData >> x >> y >> z;

			mesh->m_tangentBufferCPU[i] = XMFLOAT3(x, y, z);
		}

		int subMeshCount;
		sData >> subMeshCount;
		mesh->m_subMeshes.resize(subMeshCount);
		for (int i = 0; i < subMeshCount; i++) {
			int indexBufferSize;
			sData >> indexBufferSize;
			mesh->m_subMeshes[i].m_indexBufferCPU.resize(indexBufferSize);
			for (int j = 0; j < indexBufferSize; j++) {
				sData >> mesh->m_subMeshes[i].m_indexBufferCPU[j];
			}
		}

		for (int i = 0; i < subMeshCount; i++) {
			mesh->m_indexBufferCPU.insert(mesh->m_indexBufferCPU.end(), mesh->m_subMeshes[i].m_indexBufferCPU.begin(), mesh->m_subMeshes[i].m_indexBufferCPU.end());
		}

		//int a = 1;
		return true;
	}
}

}
#endif