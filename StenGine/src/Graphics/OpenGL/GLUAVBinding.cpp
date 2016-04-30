#include "Graphics/OpenGL/GLUAVBinding.h"
#include "Graphics/Abstraction/RendererBase.h"

#pragma warning(disable:4267)

extern PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;

namespace StenGine
{

GLUAVBinding::GLUAVBinding()
{
	m_UAVs.fill(0);
}

void GLUAVBinding::AddUAV(GLuint UAV, uint32_t index)
{
	m_UAVs[index] = UAV;
}

void GLUAVBinding::Bind()
{
	for (uint32_t i = 0; i < m_UAVs.size(); ++i)
	{
		glBindImageTexture(i, m_UAVs[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
	}
}

void GLUAVBinding::Unbind()
{

}

}