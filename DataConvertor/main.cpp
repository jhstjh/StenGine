#include "FbxReaderSG.h"
#include <iostream>
#include <vector>
#include <fstream>

std::wstring ExtractFileName(std::wstring path) {
	std::wstring name = L"mgs";

	bool ext = false;
	for (int i = path.length() - 1; i >= 0; --i) {
		if (!ext) {
			if (path[i] == '.') {
				ext = true;
			}
			else
				continue;
		}

		if (path[i] == '/' || path[i] == '\\') 
			break;
		name += path[i];
	}
	std::reverse(name.begin(), name.end());
	return L"../StenGine/StenGine_Android/StenGine_Android.Packaging/assets/Models/" + name;
}

struct DataCell {
	std::wstring filename;
	Mesh* mesh;
};

std::vector<DataCell> _data;

void LoadMesh(const std::wstring& filename) {
	Mesh* mesh = new Mesh(2);
	
	bool result = FbxReaderSG::Read(filename, mesh);
	
	std::wstring name = ExtractFileName(filename);

	DataCell model = { name, mesh };
	_data.push_back(model);
}

void DumpAllMeshData() {
	for (auto &cell : _data) {
		std::fstream fs;
		fs.open(cell.filename, std::fstream::out);

		Mesh* mesh = cell.mesh;

		// dump position buffer
		fs << mesh->m_positionBufferCPU.size() << std::endl;
		for (int i = 0; i < mesh->m_positionBufferCPU.size(); i++) {
			fs  << mesh->m_positionBufferCPU[i].x << " " 
			    << mesh->m_positionBufferCPU[i].y << " " 
			    << mesh->m_positionBufferCPU[i].z << std::endl;
		}

		// dump normal buffer
		fs << mesh->m_normalBufferCPU.size() << std::endl;
		for (int i = 0; i < mesh->m_normalBufferCPU.size(); i++) {
			fs  << mesh->m_normalBufferCPU[i].x << " "
				<< mesh->m_normalBufferCPU[i].y << " "
				<< mesh->m_normalBufferCPU[i].z << std::endl;
		}

		// dump tangent buffer
		fs << mesh->m_tangentBufferCPU.size() << std::endl;
		for (int i = 0; i < mesh->m_tangentBufferCPU.size(); i++) {
			fs  << mesh->m_tangentBufferCPU[i].x << " "
				<< mesh->m_tangentBufferCPU[i].y << " "
				<< mesh->m_tangentBufferCPU[i].z << std::endl;
		}

		// dump index buffer for each submesh
		fs << mesh->m_subMeshes.size() << std::endl;
		for (int i = 0; i < mesh->m_subMeshes.size(); i++) {
			fs << mesh->m_subMeshes[i].m_indexBufferCPU.size() << std::endl;
			for (int j = 0; j < mesh->m_subMeshes[i].m_indexBufferCPU.size(); j++) {
				fs << mesh->m_subMeshes[i].m_indexBufferCPU[j] << " ";
			}
		}

		fs.close();
	}
}

int main(int argc, char** argv) {
	LoadMesh(L"../StenGine/Model/earth.fbx");
	LoadMesh(L"../StenGine/Model/plants.fbx");

	DumpAllMeshData();

	return 0;
}