#pragma once

#include "glew.h"
#include "Graphics/Abstraction/UAVBinding.h"
#include "Graphics/D3DIncludes.h"
#include <array>

namespace StenGine
{

class GLUAVBinding : public UAVBindingImpl
{
public:
	GLUAVBinding();
	virtual void AddUAV(void* UAV, uint32_t index);
	virtual void Bind();
	virtual void Unbind();

private:
	std::array<GLuint, 2> m_UAVs; // size todo
};

//using UAVBinding = GLUAVBinding;

}