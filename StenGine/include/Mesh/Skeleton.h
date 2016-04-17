#pragma once
#include "Graphics/D3DIncludes.h"

using namespace DirectX;

namespace StenGine
{

class Joint {
	XMMATRIX m_parentJointSpaceTransform;
	int m_index;
	int m_parentIdx;
	char m_name[256];
};

class Skeleton {
public:
	Skeleton();
	~Skeleton();

	void PrepareMatrixPalette();
	inline std::vector<XMMATRIX>* GetMatrixPalette() { return &m_matrixPalette; }

private:
	//Joint* m_root;
	std::vector<Joint> m_joints;
	std::vector<XMMATRIX> m_matrixPalette;
};

}