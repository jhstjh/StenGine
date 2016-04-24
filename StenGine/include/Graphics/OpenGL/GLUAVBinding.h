#pragma once

#include "glew.h"
#include "Graphics/D3DIncludes.h"
#include <array>

namespace StenGine
{

class GLUAVBinding
{
public:
	GLUAVBinding();
	void AddUAV(GLuint UAV, uint32_t index);
	void Bind();
	void Unbind();

private:
	std::array<GLuint, 2> m_UAVs; // size todo
};

using UAVBinding = GLUAVBinding;

}