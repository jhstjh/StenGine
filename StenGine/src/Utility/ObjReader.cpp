#include "stdafx.h"

#include "Utility/ObjReader.h"

#if 0

namespace StenGine
{

void ObjReader::Read(const std::wstring& filename, MeshRenderer* mesh) {

	std::ifstream fin(filename);
	char line[256];
	std::string numString[3];

	while (fin.getline(line, 256)) {
		numString[0].clear();
		numString[1].clear();
		numString[2].clear();
		int idx = 0;
		if (line[0] == '#' || line[0] == '\n') {
			ZeroMemory(line, 256);
			continue;
		}
		if (line[0] == 'v' && line[1] == ' ') { // vertex buffer
			char* ptr = line;
			ptr += 2;
			while (*ptr != '\0') {
				while (*ptr != ' ' && *ptr != '\0') {
					numString[idx] += *ptr;
					ptr++;
				}
				idx++;
				ptr++;
			}
			mesh->m_positionBufferCPU.emplace_back(Vec3{ std::stof(numString[0]), std::stof(numString[1]), std::stof(numString[2]) });
			ZeroMemory(line, 256);
		}
		else if (line[1] == 't') { // uv buffer

		}
		else if (line[1] == 'n') { // normal buffer
			char* ptr = line;
			ptr += 3;
			while (*ptr != '\0') {
				while (*ptr != ' ' && *ptr != '\0') {
					numString[idx] += *ptr;
					ptr++;
				}
				idx++;
				ptr++;
			}
			mesh->m_colorBufferCPU.emplace_back(Vec4(std::stof(numString[0]), std::stof(numString[1]), std::stof(numString[2]), 1.0f));
			//mesh->m_colorBufferCPU.push_back((const float*)&Colors::Red);
			ZeroMemory(line, 256);
		}
		else if (line[0] == 'f') { // index buffer
			char* ptr = line;
			ptr += 2;
			while (*ptr != '\0') {
				while (*ptr != ' ' && *ptr != '\0' && *ptr != '/') {
					numString[idx] += *ptr;
					ptr++;
				}
				mesh->m_indexBufferCPU.push_back(std::stoi(numString[0]));
				numString[0].clear();
				//ptr++;
				while (*ptr != ' ' && *ptr != '\0')
					ptr++;
				ptr++;
			}
			ZeroMemory(line, 256);
		}
		else continue;

	}

}

}

#endif