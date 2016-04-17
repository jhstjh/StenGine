#pragma once

#include "Graphics/Abstraction/ConstantBufferBase.h"
#include "Graphics/D3DIncludes.h"

namespace StenGine
{

class D3D11ConstantBuffer : public ConstantBufferBase<D3D11ConstantBuffer>
{
	friend class ConstantBufferBase<D3D11ConstantBuffer>;

public:
	D3D11ConstantBuffer(uint32_t index, uint32_t size, void* bufferName);
	~D3D11ConstantBuffer();

	D3D11ConstantBuffer(D3D11ConstantBuffer&& other);
	D3D11ConstantBuffer& operator=(D3D11ConstantBuffer&& other);

protected:
	void* ImplGetBuffer();
	void ImplBind();
	// void Unbind(); need to impl this for D3D11

private:
	void* m_data;
	uint32_t m_size;
	uint32_t m_index;
	ID3D11Buffer* m_bufferName;
};

using ConstantBuffer = D3D11ConstantBuffer;

}