﻿#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/Abstraction/RendererBase.h"

#include <Windows.h>

#if GRAPHICS_D3D11
#include "D3DCompiler.h"

#define READ_SHADER_FROM_FILE 0
#if READ_SHADER_FROM_FILE
#define EXT L".cso"
#else
#define EXT L".hlsl"
#endif
#else
#define EXT L".glsl"
#endif

#include "imgui.h"

#pragma warning( disable : 4996 )

namespace StenGine
{

#if PLATFORM_WIN32

Effect::Effect(const std::wstring& filename)
{

}

Effect::Effect(const std::wstring& vsPath,
			   const std::wstring& psPath,
			   const std::wstring& gsPath = L"",
			   const std::wstring& hsPath = L"",
			   const std::wstring& dsPath = L"",
			   const std::wstring& csPath = L"")
			   : m_vertexShader(0)
			   , m_pixelShader(0)
			   , m_geometryShader(0)
			   , m_hullShader(0)
			   , m_domainShader(0)
			   , m_computeShader(0)
			   , m_inputLayout(0)
#if GRAPHICS_D3D11
			   , m_vsBlob(nullptr)
			   , m_psBlob(nullptr)
			   , m_gsBlob(nullptr)
			   , m_hsBlob(nullptr)
			   , m_dsBlob(nullptr)
			   , m_csBlob(nullptr)
			   , m_shaderResources(nullptr)
#else
			   // gl
#endif
{
#if GRAPHICS_D3D11
	// Add error checking
	HRESULT hr;

	if (vsPath.length()) {
		ReadShaderFile(vsPath, &m_vsBlob, "vs_5_0");
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateVertexShader(
			m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(),
			nullptr,
			&m_vertexShader
		);
		assert(SUCCEEDED(hr));
	}

	if (psPath.length()) {
		ReadShaderFile(psPath, &m_psBlob, "ps_5_0");
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreatePixelShader(
			m_psBlob->GetBufferPointer(),
			m_psBlob->GetBufferSize(),
			nullptr,
			&m_pixelShader
		);
		assert(SUCCEEDED(hr));
	}

	if (gsPath.length()) {
		ReadShaderFile(gsPath, &m_gsBlob, "gs_5_0");
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateGeometryShader(
			m_gsBlob->GetBufferPointer(),
			m_gsBlob->GetBufferSize(),
			nullptr,
			&m_geometryShader
			);
		assert(SUCCEEDED(hr));
	}

	if (hsPath.length()) {
		ReadShaderFile(hsPath, &m_hsBlob, "hs_5_0");
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateHullShader(
			m_hsBlob->GetBufferPointer(),
			m_hsBlob->GetBufferSize(),
			nullptr,
			&m_hullShader
			);
		assert(SUCCEEDED(hr));
	}

	if (dsPath.length()) {
		ReadShaderFile(dsPath, &m_dsBlob, "ds_5_0");
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateDomainShader(
			m_dsBlob->GetBufferPointer(),
			m_dsBlob->GetBufferSize(),
			nullptr,
			&m_domainShader
			);
		assert(SUCCEEDED(hr));
	}

	if (csPath.length()) {
		ReadShaderFile(csPath, &m_csBlob, "cs_5_0");
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateComputeShader(
			m_csBlob->GetBufferPointer(),
			m_csBlob->GetBufferSize(),
			nullptr,
			&m_computeShader
			);
		assert(SUCCEEDED(hr));
	}
#else
	char shaderbuffer[1024*256];
	const GLchar* p;
	int params = -1;

	if (vsPath.length()) {
		(ReadShaderFile(vsPath, shaderbuffer, 1024*256));
		m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
		p = (const GLchar*)shaderbuffer;
		glShaderSource(m_vertexShader, 1, &p, NULL);
		glCompileShader(m_vertexShader);

		/* check for shader compile errors - very important! */

		glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &params);
		if (GL_TRUE != params) {
			GLint maxLength = 0;
			glGetShaderiv(m_vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			//The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(m_vertexShader, maxLength, &maxLength, &infoLog[0]);
			//We don't need the shader anymore.
			glDeleteShader(m_vertexShader);

			OutputDebugStringA(&infoLog[0]);
			assert(false);
			return;

// 			fprintf(stderr, "ERROR: GL shader index %i did not compile\n", m_vertexShader);
// 			_print_shader_info_log(vs);
		}
	}

	if (hsPath.length()) {
		(ReadShaderFile(hsPath, shaderbuffer, 1024 * 256));
		m_hullShader = glCreateShader(GL_TESS_CONTROL_SHADER);
		p = (const GLchar*)shaderbuffer;
		glShaderSource(m_hullShader, 1, &p, NULL);
		glCompileShader(m_hullShader);

		/* check for shader compile errors - very important! */

		glGetShaderiv(m_hullShader, GL_COMPILE_STATUS, &params);
		if (GL_TRUE != params) {
			GLint maxLength = 0;
			glGetShaderiv(m_hullShader, GL_INFO_LOG_LENGTH, &maxLength);

			//The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(m_hullShader, maxLength, &maxLength, &infoLog[0]);
			//We don't need the shader anymore.
			glDeleteShader(m_hullShader);

			OutputDebugStringA(&infoLog[0]);
			assert(false);
			return;

			// 			fprintf(stderr, "ERROR: GL shader index %i did not compile\n", m_vertexShader);
			// 			_print_shader_info_log(vs);
		}
	}

	if (dsPath.length()) {
		(ReadShaderFile(dsPath, shaderbuffer, 1024 * 256));
		m_domainShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
		p = (const GLchar*)shaderbuffer;
		glShaderSource(m_domainShader, 1, &p, NULL);
		glCompileShader(m_domainShader);

		/* check for shader compile errors - very important! */

		glGetShaderiv(m_domainShader, GL_COMPILE_STATUS, &params);
		if (GL_TRUE != params) {
			GLint maxLength = 0;
			glGetShaderiv(m_domainShader, GL_INFO_LOG_LENGTH, &maxLength);

			//The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(m_domainShader, maxLength, &maxLength, &infoLog[0]);
			//We don't need the shader anymore.
			glDeleteShader(m_domainShader);

			OutputDebugStringA(&infoLog[0]);
			assert(false);
			return;

			// 			fprintf(stderr, "ERROR: GL shader index %i did not compile\n", m_vertexShader);
			// 			_print_shader_info_log(vs);
		}
	}

	if (psPath.length()) {
		(ReadShaderFile(psPath, shaderbuffer, 1024 * 256));
		m_pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
		p = (const GLchar*)shaderbuffer;
		glShaderSource(m_pixelShader, 1, &p, NULL);
		glCompileShader(m_pixelShader);

		/* check for shader compile errors - very important! */

		glGetShaderiv(m_pixelShader, GL_COMPILE_STATUS, &params);
		if (GL_TRUE != params) {
			//assert(false);
			// 			fprintf(stderr, "ERROR: GL shader index %i did not compile\n", m_vertexShader);
			// 			_print_shader_info_log(vs);

			GLint maxLength = 0;
			glGetShaderiv(m_pixelShader, GL_INFO_LOG_LENGTH, &maxLength);

			//The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(m_pixelShader, maxLength, &maxLength, &infoLog[0]);
			//We don't need the shader anymore.
			glDeleteShader(m_pixelShader);

			OutputDebugStringA(&infoLog[0]);
			assert(false);
			return;
		}
	}

	if (csPath.length()) {
		(ReadShaderFile(csPath, shaderbuffer, 1024 * 256));
		m_computeShader = glCreateShader(GL_COMPUTE_SHADER);
		p = (const GLchar*)shaderbuffer;
		glShaderSource(m_computeShader, 1, &p, NULL);
		glCompileShader(m_computeShader);

		/* check for shader compile errors - very important! */

		glGetShaderiv(m_computeShader, GL_COMPILE_STATUS, &params);
		if (GL_TRUE != params) {
			//assert(false);
			// 			fprintf(stderr, "ERROR: GL shader index %i did not compile\n", m_vertexShader);
			// 			_print_shader_info_log(vs);

			GLint maxLength = 0;
			glGetShaderiv(m_computeShader, GL_INFO_LOG_LENGTH, &maxLength);

			//The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(m_computeShader, maxLength, &maxLength, &infoLog[0]);
			//We don't need the shader anymore.
			glDeleteShader(m_computeShader);

			OutputDebugStringA(&infoLog[0]);
			assert(false);
			return;
		}
	}

	m_shaderProgram = glCreateProgram();
	if (vsPath.length()) glAttachShader(m_shaderProgram, m_vertexShader);
	if (hsPath.length()) glAttachShader(m_shaderProgram, m_hullShader);
	if (dsPath.length()) glAttachShader(m_shaderProgram, m_domainShader);
	if (psPath.length()) glAttachShader(m_shaderProgram, m_pixelShader);
	if (csPath.length()) glAttachShader(m_shaderProgram, m_computeShader);
	glLinkProgram(m_shaderProgram);

	/* check for shader linking errors - very important! */
	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
		//assert(false);
// 		fprintf(
// 			stderr,
// 			"ERROR: could not link shader programme GL index %i\n",
// 			shader_programme
// 			);
// 		_print_programme_info_log(shader_programme);
// 		return 1;
		GLint maxLength = 0;
		glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(m_shaderProgram, maxLength, &maxLength, &infoLog[0]);
		//We don't need the shader anymore.
		glDeleteProgram(m_shaderProgram);

		OutputDebugStringA(&infoLog[0]);
		assert(GL_TRUE != params);
		return;
	}

	glValidateProgram(m_shaderProgram);
	glGetProgramiv(m_shaderProgram, GL_VALIDATE_STATUS, &params);
	printf("program %i GL_VALIDATE_STATUS = %i\n", m_shaderProgram, params);
	if (GL_TRUE != params) {
		assert(false);
// 		_print_programme_info_log(sp);
// 		return false;
	}
#endif
}

Effect::~Effect()
{
#if GRAPHICS_D3D11
	SafeDeleteArray(m_shaderResources);
	ReleaseCOM(m_vertexShader);
	ReleaseCOM(m_pixelShader);
	ReleaseCOM(m_geometryShader);
	ReleaseCOM(m_hullShader);
	ReleaseCOM(m_domainShader);
	ReleaseCOM(m_computeShader);
#else
	// gl
#endif
}

void Effect::UnBindConstantBuffer() {
#if GRAPHICS_D3D11
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 0, nullptr);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(0, 0, nullptr);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetConstantBuffers(0, 0, nullptr);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetConstantBuffers(0, 0, nullptr);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->GSSetConstantBuffers(0, 0, nullptr);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetConstantBuffers(0, 0, nullptr);
#else
// gl
#endif
}

void Effect::UnBindShaderResource() {
#if GRAPHICS_D3D11
	static ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShaderResources(0, 16, nullSRV);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShaderResources(0, 16, nullSRV);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetShaderResources(0, 16, nullSRV);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetShaderResources(0, 16, nullSRV);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->GSSetShaderResources(0, 16, nullSRV);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetShaderResources(0, 16, nullSRV);
#else
// gl
#endif
}



void Effect::SetShader() {
#if GRAPHICS_D3D11
	//if (m_vertexShader)
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShader(m_vertexShader, 0, 0);
	//if (m_pixelShader)
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShader(m_pixelShader, 0, 0);
	//if (m_geometryShader)
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->GSSetShader(m_geometryShader, 0, 0);
	//if (m_hullShader)
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetShader(m_hullShader, 0, 0);
	//if (m_domainShader)
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetShader(m_domainShader, 0, 0);
		//if (m_domainShader)
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetShader(m_computeShader, 0, 0);
#else
	// gl
	glUseProgram(m_shaderProgram);
#endif
}

void Effect::UnSetShader() {
#if GRAPHICS_D3D11
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShader(nullptr, 0, 0);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShader(nullptr, 0, 0);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetShader(nullptr, 0, 0);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->GSSetShader(nullptr, 0, 0);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetShader(nullptr, 0, 0);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetShader(nullptr, 0, 0);
#else
	// gl
#endif
}

#if GRAPHICS_D3D11
void Effect::UnbindUnorderedAccessViews() {
	static ID3D11UnorderedAccessView* nullUAV[7] = { 0 };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetUnorderedAccessViews(0, 7, nullUAV, 0);
}

void Effect::ReadShaderFile(std::wstring filename, ID3DBlob **blob, char* target, char* entryPoint) {
	HRESULT hr;
#if !READ_SHADER_FROM_FILE
	hr = D3DCompileFromFile(
		filename.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint,
		target,
		D3DCOMPILE_DEBUG,
		0,
		blob,
		nullptr
		);
#else
	hr = D3DReadFileToBlob(vsPath.c_str(), &m_vsBlob);
#endif
	assert(SUCCEEDED(hr));
}

void Effect::SetShaderResources(ID3D11ShaderResourceView* res, int idx) {
	m_shaderResources[idx] = res;
}

ID3D11ShaderResourceView* Effect::GetOutputShaderResource(int idx) {
	return m_outputShaderResources[idx];
}
#endif


//----------------------------------------------------------//


ShadowMapEffect::ShadowMapEffect(const std::wstring& filename)
#if GRAPHICS_D3D11
	: Effect(filename + L"_vs" + EXT, L"")
#else
	: Effect(filename + L"_vs" + EXT, std::wstring(L"FX/ZOnly_ps") + EXT)
#endif
{
#if GRAPHICS_D3D11
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PEROBJ_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, nullptr,
			&m_perObjectCB);

		assert(SUCCEEDED(hr));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
#else
	glCreateVertexArrays(1, &m_inputLayout);
	glEnableVertexArrayAttrib(m_inputLayout, 0);
	glVertexArrayAttribFormat(m_inputLayout, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex::StdMeshVertex, Pos));
	glVertexArrayAttribBinding(m_inputLayout, 0, 0);

	glCreateBuffers(1, &m_perObjectCB);
	glNamedBufferStorage(m_perObjectCB, sizeof(PEROBJ_CONSTANT_BUFFER), nullptr, GL_MAP_WRITE_BIT);
#endif
}

ShadowMapEffect::~ShadowMapEffect()
{
#if GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#else
	glDeleteVertexArrays(1, &m_inputLayout);
#endif
}


//------------------------------------------------------------//

DeferredGeometryPassEffect::DeferredGeometryPassEffect(const std::wstring& vsPath, const std::wstring& psPath, const std::wstring& gsPath, const std::wstring& hsPath, const std::wstring& dsPath)
	:Effect(vsPath, psPath, gsPath, hsPath, dsPath)
{

}

DeferredGeometryPassEffect::DeferredGeometryPassEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + EXT, filename + L"_ps" + EXT)
{
	PrepareBuffer();
}

DeferredGeometryPassEffect::~DeferredGeometryPassEffect()
{
#if GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#else
	glDeleteVertexArrays(1, &m_inputLayout);
#endif
}

void DeferredGeometryPassEffect::PrepareBuffer()
{
#if GRAPHICS_D3D11
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 4, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	m_shaderResources = new ID3D11ShaderResourceView*[5];

	for (int i = 0; i < 5; i++) {
		m_shaderResources[i] = 0;
	}

	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PEROBJ_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, nullptr,
			&m_perObjectCB);

		assert(SUCCEEDED(hr));
	}

	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PERFRAME_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, nullptr,
			&m_perFrameCB);

		assert(SUCCEEDED(hr));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
#else

	glCreateVertexArrays(1, &m_inputLayout);

	glEnableVertexArrayAttrib(m_inputLayout, 0);
	glEnableVertexArrayAttrib(m_inputLayout, 1);
	glEnableVertexArrayAttrib(m_inputLayout, 2);
	glEnableVertexArrayAttrib(m_inputLayout, 3);

	glVertexArrayAttribFormat(m_inputLayout, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex::StdMeshVertex, Pos));
	glVertexArrayAttribFormat(m_inputLayout, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex::StdMeshVertex, Normal));
	glVertexArrayAttribFormat(m_inputLayout, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex::StdMeshVertex, Tangent));
	glVertexArrayAttribFormat(m_inputLayout, 3, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex::StdMeshVertex, TexUV));

	glVertexArrayAttribBinding(m_inputLayout, 0, 0);
	glVertexArrayAttribBinding(m_inputLayout, 1, 0);
	glVertexArrayAttribBinding(m_inputLayout, 2, 0);
	glVertexArrayAttribBinding(m_inputLayout, 3, 0);


	glCreateBuffers(1, &m_perFrameCB);
	glNamedBufferStorage(m_perFrameCB, sizeof(PERFRAME_CONSTANT_BUFFER), nullptr, GL_MAP_WRITE_BIT);

	glCreateBuffers(1, &m_perObjectCB);
	glNamedBufferStorage(m_perObjectCB, sizeof(PEROBJ_CONSTANT_BUFFER), nullptr, GL_MAP_WRITE_BIT);

	GLint perFrameUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerFrame");
	glUniformBlockBinding(m_shaderProgram, perFrameUBOPos, 1);

	GLint perObjUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerObj");
	glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);
#endif
}

//------------------------------------------------------------//

DeferredGeometrySkinnedPassEffect::DeferredGeometrySkinnedPassEffect(const std::wstring& vsPath, const std::wstring& psPath, const std::wstring& gsPath, const std::wstring& hsPath, const std::wstring& dsPath)
	:Effect(vsPath, psPath, gsPath, hsPath, dsPath)
{

}

DeferredGeometrySkinnedPassEffect::DeferredGeometrySkinnedPassEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + EXT, std::wstring(L"DeferredGeometryPass_vs") + EXT)
{
#if GRAPHICS_D3D11
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "JOINTWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "JOINTINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 6, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	m_shaderResources = new ID3D11ShaderResourceView*[5];

	for (int i = 0; i < 5; i++) {
		m_shaderResources[i] = 0;
	}

	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PEROBJ_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &m_perObjConstantBuffer;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, &InitData,
			&m_perObjectCB);

		assert(SUCCEEDED(hr));
	}

	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PERFRAME_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &m_perFrameConstantBuffer;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, &InitData,
			&m_perFrameCB);

		assert(SUCCEEDED(hr));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
#else

	glGenBuffers(1, &m_perFrameUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_perFrameUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PERFRAME_UNIFORM_BUFFER), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &m_perObjectUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_perObjectUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PEROBJ_UNIFORM_BUFFER), NULL, GL_DYNAMIC_DRAW);


	GLuint perFrameUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerFrame");
	glUniformBlockBinding(m_shaderProgram, perFrameUBOPos, 1);

	GLint perObjUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerObj");
	glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);

	DiffuseMapPosition = glGetUniformLocation(m_shaderProgram, "gDiffuseMap");
	NormalMapPosition = glGetUniformLocation(m_shaderProgram, "gNormalMap");
	ShadowMapPosition = glGetUniformLocation(m_shaderProgram, "gShadowMap");
	CubeMapPosition = glGetUniformLocation(m_shaderProgram, "gCubeMap");
#endif
}

DeferredGeometrySkinnedPassEffect::~DeferredGeometrySkinnedPassEffect()
{
#if GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}

void DeferredGeometrySkinnedPassEffect::UpdateConstantBuffer() {
#if GRAPHICS_D3D11
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_perObjectCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perObjConstantBuffer, sizeof(PEROBJ_CONSTANT_BUFFER));
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_perObjectCB, NULL);
	}

	{
		D3D11_MAPPED_SUBRESOURCE ms;
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_perFrameCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perFrameConstantBuffer, sizeof(PERFRAME_CONSTANT_BUFFER));
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_perFrameCB, NULL);
	}
#else
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_perObjectUBO);
	PEROBJ_UNIFORM_BUFFER* perObjUBOPtr = (PEROBJ_UNIFORM_BUFFER*)glMapBufferRange(
		GL_UNIFORM_BUFFER,
		0,
		sizeof(PEROBJ_UNIFORM_BUFFER),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
		);
	memcpy(perObjUBOPtr, &m_perObjUniformBuffer, sizeof(PEROBJ_UNIFORM_BUFFER));
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_perFrameUBO);
	PERFRAME_UNIFORM_BUFFER* perFrameUBOPtr = (PERFRAME_UNIFORM_BUFFER*)glMapBufferRange(
		GL_UNIFORM_BUFFER,
		0,
		sizeof(PERFRAME_UNIFORM_BUFFER),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
		);
	memcpy(perFrameUBOPtr, &m_perFrameUniformBuffer, sizeof(PERFRAME_UNIFORM_BUFFER));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
#endif
}

void DeferredGeometrySkinnedPassEffect::BindConstantBuffer() {
#if GRAPHICS_D3D11
	ID3D11Buffer* cbuf[] = { m_perObjectCB, m_perFrameCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 2, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(0, 2, cbuf);
#endif
}

void DeferredGeometrySkinnedPassEffect::BindShaderResource() {
#if GRAPHICS_D3D11
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShaderResources(0, 5, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShaderResources(0, 5, m_shaderResources);
#else
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DiffuseMap);
	glUniform1i(DiffuseMapPosition, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NormalMap);
	glUniform1i(NormalMapPosition, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ShadowMapTex);
	glUniform1i(ShadowMapPosition, 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapTex);
	glUniform1i(CubeMapPosition, 3);
#endif
}

//----------------------------------------------------------//


DeferredShadingPassEffect::DeferredShadingPassEffect(const std::wstring& filename)
	: Effect(std::wstring(L"FX/ScreenQuad_vs") + EXT, filename + L"_ps" + EXT)
{
#if GRAPHICS_D3D11
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	m_shaderResources = new ID3D11ShaderResourceView*[5];
	for (int i = 0; i < 5; i++) {
		m_shaderResources[i] = 0;
	}

	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PERFRAME_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, nullptr,
			&m_perFrameCB);

		assert(SUCCEEDED(hr));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
#else
	glCreateBuffers(1, &m_perFrameCB);
	glNamedBufferStorage(m_perFrameCB, sizeof(PERFRAME_CONSTANT_BUFFER), nullptr, GL_MAP_WRITE_BIT);
#endif
}

DeferredShadingPassEffect::~DeferredShadingPassEffect()
{
#if GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}

//----------------------------------------------------------//


GodRayEffect::GodRayEffect(const std::wstring& filename)
	: Effect(std::wstring(L"FX/ScreenQuad_vs") + EXT, filename + L"_ps" + EXT)
{
#if GRAPHICS_D3D11
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	m_shaderResources = new ID3D11ShaderResourceView*[1];
	for (int i = 0; i < 1; i++) {
		m_shaderResources[i] = 0;
	}

	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PERFRAME_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &m_perFrameConstantBuffer;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, &InitData,
			&m_perFrameCB);

		assert(SUCCEEDED(hr));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
#else
	glGenBuffers(1, &m_perFrameUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_perFrameUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PERFRAME_UNIFORM_BUFFER), NULL, GL_DYNAMIC_DRAW);

	OcclusionMapPosition = glGetUniformLocation(m_shaderProgram, "gOcclusionGMap");
#endif
}

GodRayEffect::~GodRayEffect()
{
#if GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}

void GodRayEffect::UpdateConstantBuffer() {
#if GRAPHICS_D3D11
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_perFrameCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perFrameConstantBuffer, sizeof(PERFRAME_CONSTANT_BUFFER));
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_perFrameCB, NULL);
	}
#else
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_perFrameUBO);
	PERFRAME_UNIFORM_BUFFER* perFrameUBOPtr = (PERFRAME_UNIFORM_BUFFER*)glMapBufferRange(
		GL_UNIFORM_BUFFER,
		0,
		sizeof(PERFRAME_UNIFORM_BUFFER),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
		);
	memcpy(perFrameUBOPtr, &m_perFrameUniformBuffer, sizeof(PERFRAME_UNIFORM_BUFFER));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
#endif
}

void GodRayEffect::BindConstantBuffer() {
#if GRAPHICS_D3D11
	ID3D11Buffer* cbuf[] = { m_perFrameCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(1, 1, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(1, 1, cbuf);
#endif
}

void GodRayEffect::BindShaderResource() {
#if GRAPHICS_D3D11
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShaderResources(0, 1, m_shaderResources);
#else
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, OcclusionMap);
	glUniform1i(OcclusionMapPosition, 0);
#endif
}

//----------------------------------------------------------//

SkyboxEffect::SkyboxEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + EXT, filename + L"_ps" + EXT)
{
#if GRAPHICS_D3D11
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	m_shaderResources = new ID3D11ShaderResourceView*[1];
	for (int i = 0; i < 1; i++) {
		m_shaderResources[i] = 0;
	}

	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PEROBJ_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, nullptr,
			&m_perObjectCB);

		assert(SUCCEEDED(hr));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
#else
	glCreateBuffers(1, &m_perObjectCB);
	glNamedBufferStorage(m_perObjectCB, sizeof(PEROBJ_CONSTANT_BUFFER), nullptr, GL_MAP_WRITE_BIT);
#endif
}

SkyboxEffect::~SkyboxEffect()
{
#if GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}

//------------------------------------------------------------//

DebugLineEffect::DebugLineEffect(const std::wstring& vsPath, const std::wstring& psPath, const std::wstring& gsPath, const std::wstring& hsPath, const std::wstring& dsPath)
	:Effect(vsPath, psPath, gsPath, hsPath, dsPath)
{

}

DebugLineEffect::DebugLineEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + EXT, filename + L"_ps" + EXT)
{
#if GRAPHICS_D3D11
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PEROBJ_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &m_perObjConstantBuffer;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, &InitData,
			&m_perObjectCB);

		assert(SUCCEEDED(hr));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
#else
	glGenBuffers(1, &m_perObjectUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_perObjectUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PEROBJ_UNIFORM_BUFFER), NULL, GL_DYNAMIC_DRAW);
#endif
}

DebugLineEffect::~DebugLineEffect()
{
#if GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}

void DebugLineEffect::UpdateConstantBuffer() {
#if GRAPHICS_D3D11
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_perObjectCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perObjConstantBuffer, sizeof(PEROBJ_CONSTANT_BUFFER));
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_perObjectCB, NULL);
	}
#else
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_perObjectUBO);
	PEROBJ_UNIFORM_BUFFER* perObjectUBOPtr = (PEROBJ_UNIFORM_BUFFER*)glMapBufferRange(
		GL_UNIFORM_BUFFER,
		0,
		sizeof(PEROBJ_UNIFORM_BUFFER),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
	);
	memcpy(perObjectUBOPtr, &m_perObjUniformBuffer, sizeof(PEROBJ_UNIFORM_BUFFER));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
#endif
	}

void DebugLineEffect::BindConstantBuffer() {
#if GRAPHICS_D3D11
	ID3D11Buffer* cbuf[] = { m_perObjectCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 1, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(0, 1, cbuf);
#endif
}

void DebugLineEffect::BindShaderResource() {

}


//----------------------------------------------------------//

DeferredGeometryTessPassEffect::DeferredGeometryTessPassEffect(const std::wstring& filename)
	: DeferredGeometryPassEffect(
		filename + L"_vs" + EXT,
		filename + L"_ps" + EXT,
		L"",
		filename + L"_hs" + EXT,
		filename + L"_ds" + EXT)
{
	PrepareBuffer();
}

DeferredGeometryTessPassEffect::~DeferredGeometryTessPassEffect()
{
#if GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}

//----------------------------------------------------------//

DeferredGeometryTerrainPassEffect::DeferredGeometryTerrainPassEffect(const std::wstring& filename)
	: Effect(
		filename + L"_vs" + EXT,
		filename + L"_ps" + EXT,
		L"",
		filename + L"_hs" + EXT,
		filename + L"_ds" + EXT)
{

#if GRAPHICS_D3D11
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HRESULT hr = (static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 3, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	m_shaderResources = new ID3D11ShaderResourceView*[8];

	for (int i = 0; i < 8; i++) {
		m_shaderResources[i] = 0;
	}

	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PEROBJ_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Create the buffer.
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, nullptr, &m_perObjectCB));
	}

	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PERFRAME_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Create the buffer.
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, nullptr, &m_perFrameCB));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);

#endif

#if GRAPHICS_OPENGL

	glCreateVertexArrays(1, &m_inputLayout);

	glEnableVertexArrayAttrib(m_inputLayout, 0);
	glEnableVertexArrayAttrib(m_inputLayout, 1);
	glEnableVertexArrayAttrib(m_inputLayout, 2);

	glVertexArrayAttribFormat(m_inputLayout, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_inputLayout, 1, 2, GL_FLOAT, GL_FALSE, 12);
	glVertexArrayAttribFormat(m_inputLayout, 2, 2, GL_FLOAT, GL_FALSE, 20);

	glVertexArrayAttribBinding(m_inputLayout, 0, 0);
	glVertexArrayAttribBinding(m_inputLayout, 1, 0);
	glVertexArrayAttribBinding(m_inputLayout, 2, 0);



	glCreateBuffers(1, &m_perObjectCB);
	glNamedBufferStorage(m_perObjectCB, sizeof(PEROBJ_CONSTANT_BUFFER), nullptr, GL_MAP_WRITE_BIT);

	glCreateBuffers(1, &m_perFrameCB);
	glNamedBufferStorage(m_perFrameCB, sizeof(PERFRAME_CONSTANT_BUFFER), nullptr, GL_MAP_WRITE_BIT);

	GLint perFrameUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerFrame");
	glUniformBlockBinding(m_shaderProgram, perFrameUBOPos, 1);
	
	GLint perObjUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerObj");
	glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);
#endif
}

DeferredGeometryTerrainPassEffect::~DeferredGeometryTerrainPassEffect()
{
#if GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif

#if GRAPHICS_OPENGL
	glDeleteBuffers(1, &m_perObjectCB);
	glDeleteBuffers(1, &m_perFrameCB);
#endif
}


TerrainShadowMapEffect::TerrainShadowMapEffect(const std::wstring& filename)
	: Effect(
		filename + L"_vs" + EXT,
		L"",
		L"",
		filename + L"_hs" + EXT,
		filename + L"_ds" + EXT)
{
#if GRAPHICS_D3D11
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HRESULT hr = (static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 3, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	m_shaderResources = new ID3D11ShaderResourceView*[8];

	for (int i = 0; i < 8; i++) {
		m_shaderResources[i] = 0;
	}

	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PEROBJ_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Create the buffer.
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, nullptr, &m_perObjectCB));
	}

	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PERFRAME_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Create the buffer.
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, nullptr, &m_perFrameCB));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
#endif

#if GRAPHICS_OPENGL

	glCreateVertexArrays(1, &m_inputLayout);

	glEnableVertexArrayAttrib(m_inputLayout, 0);
	glEnableVertexArrayAttrib(m_inputLayout, 1);
	glEnableVertexArrayAttrib(m_inputLayout, 2);

	glVertexArrayAttribFormat(m_inputLayout, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_inputLayout, 1, 2, GL_FLOAT, GL_FALSE, 12);
	glVertexArrayAttribFormat(m_inputLayout, 2, 2, GL_FLOAT, GL_FALSE, 20);

	glVertexArrayAttribBinding(m_inputLayout, 0, 0);
	glVertexArrayAttribBinding(m_inputLayout, 1, 0);
	glVertexArrayAttribBinding(m_inputLayout, 2, 0);

	glCreateBuffers(1, &m_perObjectCB);
	glNamedBufferStorage(m_perObjectCB, sizeof(PEROBJ_CONSTANT_BUFFER), nullptr, GL_MAP_WRITE_BIT);

	glCreateBuffers(1, &m_perFrameCB);
	glNamedBufferStorage(m_perFrameCB, sizeof(PERFRAME_CONSTANT_BUFFER), nullptr, GL_MAP_WRITE_BIT);

	GLint perFrameUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerFrame");
	glUniformBlockBinding(m_shaderProgram, perFrameUBOPos, 1);

	GLint perObjUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerObj");
	glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);
#endif
}

TerrainShadowMapEffect::~TerrainShadowMapEffect()
{
#if GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif

#if GRAPHICS_OPENGL
	glDeleteBuffers(1, &m_perObjectCB);
	glDeleteBuffers(1, &m_perFrameCB);
#endif
}

//----------------------------------------------------------//

VBlurEffect::VBlurEffect(const std::wstring& filename)
	: Effect(L"", L"", L"", L"", L"", filename + L"_cs" + EXT)
{

}

//----------------------------------------------------------//

HBlurEffect::HBlurEffect(const std::wstring& filename)
	: Effect(L"", L"", L"", L"", L"", filename + L"_cs" + EXT)
{

}

//-------------------------------------------//

ImGuiEffect::ImGuiEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + EXT, filename + L"_ps" + EXT, L"", L"", L"", L"")
{
#if GRAPHICS_D3D11

#endif

#if GRAPHICS_OPENGL
	glCreateVertexArrays(1, &m_inputLayout);

	glEnableVertexArrayAttrib(m_inputLayout, 0);
	glEnableVertexArrayAttrib(m_inputLayout, 1);
	glEnableVertexArrayAttrib(m_inputLayout, 2);

	glVertexArrayAttribFormat(m_inputLayout, 0, 2, GL_FLOAT, GL_FALSE, offsetof(ImDrawVert, pos));
	glVertexArrayAttribFormat(m_inputLayout, 1, 2, GL_FLOAT, GL_FALSE, offsetof(ImDrawVert, uv));
	glVertexArrayAttribFormat(m_inputLayout, 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(ImDrawVert, col));

	glVertexArrayAttribBinding(m_inputLayout, 0, 0);
	glVertexArrayAttribBinding(m_inputLayout, 1, 0);
	glVertexArrayAttribBinding(m_inputLayout, 2, 0);

	glCreateBuffers(1, &m_imguiCB);
	glNamedBufferStorage(m_imguiCB, sizeof(IMGUI_CONSTANT_BUFFER), nullptr, GL_MAP_WRITE_BIT);

	GLint imguiUBOPos = glGetUniformBlockIndex(m_shaderProgram, "imGuiCB");
	glUniformBlockBinding(m_shaderProgram, imguiUBOPos, 0);
#endif
}

ImGuiEffect::~ImGuiEffect()
{
#if GRAPHICS_D3D11

#endif

#if GRAPHICS_OPENGL
	glDeleteBuffers(1, &m_imguiCB);
#endif
}

//-------------------------------------------//


BlurEffect::BlurEffect(const std::wstring& filename)
	: Effect(std::wstring(L"FX/ScreenQuad_vs") + EXT, filename + L"_ps" + EXT)
{
#if GRAPHICS_D3D11
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(SETTING_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, nullptr,
			&m_settingCB);

		assert(SUCCEEDED(hr));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
#endif

#if GRAPHICS_OPENGL
	glCreateBuffers(1, &m_settingCB);
	glNamedBufferStorage(m_settingCB, sizeof(SETTING_CONSTANT_BUFFER), nullptr, GL_MAP_WRITE_BIT);

	GLint settingCBPos = glGetUniformBlockIndex(m_shaderProgram, "cbSettings");
	glUniformBlockBinding(m_shaderProgram, settingCBPos, 0);
#endif
}

BlurEffect::~BlurEffect()
{
#if GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}

#if GRAPHICS_D3D11
//----------------------------------------------------------//


DeferredShadingCS::DeferredShadingCS(const std::wstring& filename)
	: Effect(L"", L"", L"", L"", L"", filename + L"_cs" + EXT)
{

	m_shaderResources = new ID3D11ShaderResourceView*[4];
	for (int i = 0; i < 4; i++) {
		m_shaderResources[i] = 0;
	}

	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(PERFRAME_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, nullptr,
			&m_perFrameCB);

		assert(SUCCEEDED(hr));
	}


	m_outputShaderResources = new ID3D11ShaderResourceView*[2];
	m_unorderedAccessViews = new ID3D11UnorderedAccessView*[2];

	for (int i = 0; i < 2; i++) {

		D3D11_TEXTURE2D_DESC shadedTexDesc;
		shadedTexDesc.Width = Renderer::Instance()->GetScreenWidth();
		shadedTexDesc.Height = Renderer::Instance()->GetScreenHeight();
		shadedTexDesc.MipLevels = 1;
		shadedTexDesc.ArraySize = 1;
		shadedTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		shadedTexDesc.SampleDesc.Count = 1;
		shadedTexDesc.SampleDesc.Quality = 0;
		shadedTexDesc.Usage = D3D11_USAGE_DEFAULT;
		shadedTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		shadedTexDesc.CPUAccessFlags = 0;
		shadedTexDesc.MiscFlags = 0;

		ID3D11Texture2D* shadedTex = 0;
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateTexture2D(&shadedTexDesc, 0, &shadedTex));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = shadedTexDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateShaderResourceView(shadedTex, &srvDesc, &m_outputShaderResources[i]));

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = shadedTexDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateUnorderedAccessView(shadedTex, &uavDesc, &m_unorderedAccessViews[i]));


		ReleaseCOM(shadedTex);

	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
}

DeferredShadingCS::~DeferredShadingCS()
{
	ReleaseCOM(m_inputLayout);
}

void DeferredShadingCS::UpdateConstantBuffer() {

	D3D11_MAPPED_SUBRESOURCE ms;
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_perFrameCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &m_perFrameConstantBuffer, sizeof(PERFRAME_CONSTANT_BUFFER));
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_perFrameCB, NULL);

}

void DeferredShadingCS::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perFrameCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetConstantBuffers(0, 1, cbuf);
}

void DeferredShadingCS::BindShaderResource(int idx) {
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetShaderResources(0, 4, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetUnorderedAccessViews(0, 2, &m_unorderedAccessViews[idx], 0);
}


//----------------------------------------------------------//


#else


/********************************************************************/


bool Effect::ReadShaderFile(std::wstring filename, char* shaderContent, int maxLength) {
	std::string s(filename.begin(), filename.end());
	const char* lFilename = s.c_str();
	
	HANDLE hfile;
	LARGE_INTEGER file_size;
	hfile = CreateFile((LPCWSTR)(filename.c_str()), GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	GetFileSizeEx(hfile, &file_size);
	
	DWORD read;
	ReadFile(hfile, shaderContent, maxLength, &read, NULL);
	
	shaderContent[read] = 0;
	
	CloseHandle(hfile);
	
	return true;
}

#endif

#else
Effect::Effect(const std::string vsPath, std::string psPath) {

	// Create shader program
	m_shaderProgram = glCreateProgram();
	LOGI("Created Shader %d", m_shaderProgram);

	// Create and compile vertex shader
	if (!ndk_helper::shader::CompileShader(&m_vertexShader, GL_VERTEX_SHADER, vsPath.c_str()))
	{
		LOGI("Failed to compile vertex shader");
		glDeleteProgram(m_shaderProgram);
		return;
	}

	// Create and compile fragment shader
	if (!ndk_helper::shader::CompileShader(&m_pixelShader, GL_FRAGMENT_SHADER, psPath.c_str()))
	{
		LOGI("Failed to compile fragment shader");
		glDeleteProgram(m_shaderProgram);
		return;
	}

	// Attach vertex shader to program
	glAttachShader(m_shaderProgram, m_vertexShader);

	// Attach fragment shader to program
	glAttachShader(m_shaderProgram, m_pixelShader);

	// 	// Bind attribute locations
	// 	// this needs to be done prior to linking
	// 	glBindAttribLocation(program, ATTRIB_VERTEX, "myVertex");
	// 	glBindAttribLocation(program, ATTRIB_NORMAL, "myNormal");
	// 	glBindAttribLocation(program, ATTRIB_UV, "myUV");

		// Link program
	if (!ndk_helper::shader::LinkProgram(m_shaderProgram))
	{
		LOGI("Failed to link program: %d", m_shaderProgram);

		if (m_vertexShader)
		{
			glDeleteShader(m_vertexShader);
			m_vertexShader = 0;
		}
		if (m_pixelShader)
		{
			glDeleteShader(m_pixelShader);
			m_pixelShader = 0;
		}
		if (m_shaderProgram)
		{
			glDeleteProgram(m_shaderProgram);
		}

		return;
	}

	// Get uniform locations
// 	params->matrix_projection_ = glGetUniformLocation(program, "uPMatrix");
// 	params->matrix_view_ = glGetUniformLocation(program, "uMVMatrix");
// 
// 	params->light0_ = glGetUniformLocation(program, "vLight0");
// 	params->material_diffuse_ = glGetUniformLocation(program, "vMaterialDiffuse");
// 	params->material_ambient_ = glGetUniformLocation(program, "vMaterialAmbient");
// 	params->material_specular_ = glGetUniformLocation(program, "vMaterialSpecular");

	// Release vertex and fragment shaders
	if (m_vertexShader)
		glDeleteShader(m_vertexShader);
	if (m_pixelShader)
		glDeleteShader(m_pixelShader);

	//	params->program_ = program;
	return;
}

void Effect::SetShader() {
	glUseProgram(m_shaderProgram);
}

/***************************************************************/

DebugLineEffect::DebugLineEffect(const std::string &filename)
	: Effect(filename + "_vs.glsl", filename + "_ps.glsl")
{
	glGenBuffers(1, &m_perObjectUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_perObjectUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PEROBJ_UNIFORM_BUFFER), NULL, GL_DYNAMIC_DRAW);
}

DebugLineEffect::~DebugLineEffect()
{

}

void DebugLineEffect::UpdateConstantBuffer() {
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_perObjectUBO);
	PEROBJ_UNIFORM_BUFFER* perObjectUBOPtr = (PEROBJ_UNIFORM_BUFFER*)glMapBufferRange(
		GL_UNIFORM_BUFFER,
		0,
		sizeof(PEROBJ_UNIFORM_BUFFER),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
	);
	memcpy(perObjectUBOPtr, &m_perObjUniformBuffer, sizeof(PEROBJ_UNIFORM_BUFFER));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void DebugLineEffect::BindConstantBuffer() {

}

#endif

//----------------------------------------------------------//

DEFINE_SINGLETON_CLASS(EffectsManager)

EffectsManager::EffectsManager()
	: m_shadowMapEffect(nullptr)
	, m_terrainShadowMapEffect(nullptr)
	, m_deferredGeometryPassEffect(nullptr)
	, m_deferredGeometrySkinnedPassEffect(nullptr)
	, m_deferredGeometryTerrainPassEffect(nullptr)
	, m_deferredGeometryTessPassEffect(nullptr)
	, m_deferredShadingPassEffect(nullptr)
	, m_deferredShadingCSEffect(nullptr)
	, m_godrayEffect(nullptr)
	, m_skyboxEffect(nullptr)
	, m_blurEffect(nullptr)
	, m_vblurEffect(nullptr)
	, m_hblurEffect(nullptr)
	, m_debugLineEffect(nullptr)
{

	m_shadowMapEffect = std::unique_ptr<ShadowMapEffect>(new ShadowMapEffect(L"FX/ShadowMap"));
	m_deferredGeometryPassEffect = std::unique_ptr<DeferredGeometryPassEffect>(new DeferredGeometryPassEffect(L"FX/DeferredGeometryPass"));
	m_deferredShadingPassEffect = std::unique_ptr<DeferredShadingPassEffect>(new DeferredShadingPassEffect(L"FX/DeferredShadingPass"));
	m_deferredGeometryTessPassEffect = std::unique_ptr<DeferredGeometryTessPassEffect>(new DeferredGeometryTessPassEffect(L"FX/DeferredGeometryTessPass"));
	m_skyboxEffect = std::unique_ptr<SkyboxEffect>(new SkyboxEffect(L"FX/Skybox"));
	m_debugLineEffect = std::unique_ptr<DebugLineEffect>(new DebugLineEffect(L"FX/DebugLine"));
	m_godrayEffect = std::unique_ptr<GodRayEffect>(new GodRayEffect(L"FX/GodRay"));
	m_deferredGeometryTerrainPassEffect = std::unique_ptr<DeferredGeometryTerrainPassEffect>(new DeferredGeometryTerrainPassEffect(L"FX/DeferredGeometryTerrainPass"));
	m_terrainShadowMapEffect = std::unique_ptr<TerrainShadowMapEffect>(new TerrainShadowMapEffect(L"FX/DeferredGeometryTerrainPass"));
	m_vblurEffect = std::unique_ptr<VBlurEffect>(new VBlurEffect(L"FX/VBlur"));
	m_hblurEffect = std::unique_ptr<HBlurEffect>(new HBlurEffect(L"FX/HBlur"));
	m_blurEffect = std::unique_ptr<BlurEffect>(new BlurEffect(L"FX/Blur"));
	m_imguiEffect = std::unique_ptr<ImGuiEffect>(new ImGuiEffect(L"FX/imgui"));

#if GRAPHICS_D3D11
	m_deferredShadingCSEffect = std::unique_ptr<DeferredShadingCS>(new DeferredShadingCS(L"FX/DeferredShading"));
#endif

}

EffectsManager::~EffectsManager()
{

}

}