#pragma once

#include "Graphics/Abstraction/ConstantBuffer.h"
#include "Graphics/D3DIncludes.h"

namespace StenGine
{

class D3D11ConstantBuffer : public ConstantBufferImpl
{
public:
	D3D11ConstantBuffer(uint32_t index, uint32_t size, void* bufferName);
	~D3D11ConstantBuffer();

	virtual void* GetBuffer();
	virtual void Bind();
	// void Unbind(); need to impl this for D3D11

private:
	void* m_data;
	uint32_t m_size;
	uint32_t m_index;
	ID3D11Buffer* m_bufferName;
};

}