#include "EffectsManager.h"

#ifndef PLATFORM_ANDROID

#include "RendererBase.h"
#include "D3DCompiler.h"

#ifdef GRAPHICS_D3D11
#define READ_SHADER_FROM_FILE 0
#if READ_SHADER_FROM_FILE
#define EXT L".cso"
#else
#define EXT L".hlsl"
#endif
#else
#define EXT L".glsl"
#endif

#endif

#pragma warning( disable : 4996 )

#ifndef PLATFORM_ANDROID

Effect::Effect(const std::wstring& filename)
{

}

Effect::Effect(const std::wstring& vsPath,
			   const std::wstring& psPath,
			   const std::wstring& gsPath = L"",
			   const std::wstring& hsPath = L"",
			   const std::wstring& dsPath = L"",
			   const std::wstring& csPath = L""):
			   m_vertexShader(0),
			   m_pixelShader(0),
			   m_geometryShader(0),
			   m_hullShader(0),
			   m_domainShader(0),
			   m_computeShader(0)
#ifdef GRAPHICS_D3D11
			   ,
			   m_vsBlob(0),
			   m_psBlob(0),
			   m_gsBlob(0),
			   m_hsBlob(0),
			   m_dsBlob(0),
			   m_csBlob(0)
#else
			   // gl
#endif
{
#ifdef GRAPHICS_D3D11
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
		assert(ReadShaderFile(vsPath, shaderbuffer, 1024*256));
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

	if (psPath.length()) {
		assert(ReadShaderFile(psPath, shaderbuffer, 1024 * 256));
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

	m_shaderProgram = glCreateProgram();
	glAttachShader(m_shaderProgram, m_pixelShader);
	glAttachShader(m_shaderProgram, m_vertexShader);
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
		assert(false);
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
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
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

#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
	: Effect(filename + L"_vs" + EXT, L"")
#else
	: Effect(filename + L"_vs" + EXT, std::wstring(L"FX/ZOnly_ps") + EXT)
#endif
{
#ifdef GRAPHICS_D3D11
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

void ShadowMapEffect::UpdateConstantBuffer() {
#ifdef GRAPHICS_D3D11
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_perObjectCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perObjConstantBuffer, sizeof(PEROBJ_CONSTANT_BUFFER));
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_perObjectCB, NULL);
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
#endif
}

void ShadowMapEffect::BindConstantBuffer() {
#ifdef GRAPHICS_D3D11
	ID3D11Buffer* cbuf[] = { m_perObjectCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 1, cbuf);
#endif
}

ShadowMapEffect::~ShadowMapEffect()
{
#ifdef GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}


//------------------------------------------------------------//

DeferredGeometryPassEffect::DeferredGeometryPassEffect(const std::wstring& vsPath, const std::wstring& psPath, const std::wstring& gsPath, const std::wstring& hsPath, const std::wstring& dsPath)
	:Effect(vsPath, psPath, gsPath, hsPath, dsPath)
#ifdef GRAPHICS_OPENGL
	, m_buffer(4096)
	, m_bufferOffset(0)
#endif
{

}

DeferredGeometryPassEffect::DeferredGeometryPassEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + EXT, filename + L"_ps" + EXT)
#ifdef GRAPHICS_OPENGL
	, m_buffer{ 1024 * 256 }
	, m_bufferOffset(0)
#endif
{
#ifdef GRAPHICS_D3D11
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

	GLint perFrameUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerFrame");
	glUniformBlockBinding(m_shaderProgram, perFrameUBOPos, 0);

	GLint perObjUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerObj");
	glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 1);

	DiffuseMapPosition = glGetUniformLocation(m_shaderProgram, "gDiffuseMap");
	NormalMapPosition = glGetUniformLocation(m_shaderProgram, "gNormalMap");
	ShadowMapPosition = glGetUniformLocation(m_shaderProgram, "gShadowMap");
	CubeMapPosition = glGetUniformLocation(m_shaderProgram, "gCubeMap");

	m_bufferBase = m_buffer.lock();
#endif
}

DeferredGeometryPassEffect::~DeferredGeometryPassEffect()
{
#ifdef GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#else
	m_buffer.unlock();
#endif
}

void DeferredGeometryPassEffect::UpdateConstantBuffer() {
#ifdef GRAPHICS_D3D11
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

	// 2nd para, should match layout binding in GLSL
	// if no binding is specified
	// call glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);
	// to specify the binding point and math 2nd param with it

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

	//m_constantBuffers.emplace_back(0, sizeof(PERFRAME_UNIFORM_BUFFER), 0, &m_perFrameUniformBuffer);
	//m_constantBuffers.emplace_back((sizeof(PERFRAME_UNIFORM_BUFFER) / 256 + 1) * 256, sizeof(PEROBJ_UNIFORM_BUFFER), 1, &m_perObjUniformBuffer);
	//// 256 is from glGetIntegerv​(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, ...);
	//
	//memcpy((uint8_t*)m_bufferBase + m_bufferOffset, &m_perFrameUniformBuffer, sizeof(PERFRAME_UNIFORM_BUFFER));
	//glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_buffer.GetBuffer(), m_bufferOffset, sizeof(PERFRAME_UNIFORM_BUFFER));
	//m_bufferOffset += ((sizeof(PERFRAME_UNIFORM_BUFFER) / 256 + 1) * 256);
	//if (m_bufferOffset >= 1024 * 256 - sizeof(PEROBJ_UNIFORM_BUFFER)) m_bufferOffset = 0;
	//
	//memcpy((uint8_t*)m_bufferBase + m_bufferOffset, &m_perObjUniformBuffer, sizeof(PEROBJ_UNIFORM_BUFFER));
	//glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_buffer.GetBuffer(), m_bufferOffset, sizeof(PEROBJ_UNIFORM_BUFFER));
	//m_bufferOffset += ((sizeof(PEROBJ_UNIFORM_BUFFER) / 256 + 1) * 256);
	//if (m_bufferOffset >= 1024 * 256 - sizeof(PERFRAME_UNIFORM_BUFFER)) m_bufferOffset = 0;

	//for (uint32_t i = 0; i < m_constantBuffers.size(); i++)
	//{
	//	memcpy((uint8_t*)m_bufferBase + m_constantBuffers[i].offset, m_constantBuffers[i].data, m_constantBuffers[i].size);
	//	glBindBufferRange(GL_UNIFORM_BUFFER, m_constantBuffers[i].pos, m_buffer.GetBuffer(), m_constantBuffers[i].offset, m_constantBuffers[i].size);
	//	auto err = glGetError();
	//	err += 0;
	//}

#endif
}

void DeferredGeometryPassEffect::MapConstantBuffer(void* bufferBase)
{
	//m_perFrameUniformBuffer = (PERFRAME_UNIFORM_BUFFER*)bufferBase;
	//m_perObjUniformBuffer = (PEROBJ_UNIFORM_BUFFER*)((uint8_t*)bufferBase + (sizeof(PERFRAME_UNIFORM_BUFFER) / 256 + 1) * 256);

	//m_constantBuffers.emplace_back(0, sizeof(PERFRAME_UNIFORM_BUFFER), 0, &m_perFrameUniformBuffer);
	//m_constantBuffers.emplace_back((sizeof(PERFRAME_UNIFORM_BUFFER) / 256 + 1) * 256, sizeof(PEROBJ_UNIFORM_BUFFER), 1, &m_perObjUniformBuffer);
}

void DeferredGeometryPassEffect::BindConstantBuffer() {
#ifdef GRAPHICS_D3D11
	ID3D11Buffer* cbuf[] = { m_perObjectCB, m_perFrameCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 2, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(0, 2, cbuf);
#endif
}

void DeferredGeometryPassEffect::BindShaderResource() {
#ifdef GRAPHICS_D3D11
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


//------------------------------------------------------------//

DeferredGeometrySkinnedPassEffect::DeferredGeometrySkinnedPassEffect(const std::wstring& vsPath, const std::wstring& psPath, const std::wstring& gsPath, const std::wstring& hsPath, const std::wstring& dsPath)
	:Effect(vsPath, psPath, gsPath, hsPath, dsPath)
{

}

DeferredGeometrySkinnedPassEffect::DeferredGeometrySkinnedPassEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + EXT, std::wstring(L"DeferredGeometryPass_vs") + EXT)
{
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}

void DeferredGeometrySkinnedPassEffect::UpdateConstantBuffer() {
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
	ID3D11Buffer* cbuf[] = { m_perObjectCB, m_perFrameCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 2, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(0, 2, cbuf);
#endif
}

void DeferredGeometrySkinnedPassEffect::BindShaderResource() {
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
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

	DiffuseGMapPosition = glGetUniformLocation(m_shaderProgram, "gDiffuseGMap");
	NormalGMapPosition = glGetUniformLocation(m_shaderProgram, "gNormalGMap");
	SpecularGMapPosition = glGetUniformLocation(m_shaderProgram, "gSpecularGMap");
	DepthGMapPosition = glGetUniformLocation(m_shaderProgram, "gDepthGMap");
#endif
}

DeferredShadingPassEffect::~DeferredShadingPassEffect()
{
#ifdef GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}

void DeferredShadingPassEffect::UpdateConstantBuffer() {
#ifdef GRAPHICS_D3D11
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

void DeferredShadingPassEffect::BindConstantBuffer() {
#ifdef GRAPHICS_D3D11
	ID3D11Buffer* cbuf[] = { m_perFrameCB };
	//static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 1, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(0, 1, cbuf);
#endif
}

void DeferredShadingPassEffect::BindShaderResource() {
#ifdef GRAPHICS_D3D11
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShaderResources(0, 5, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShaderResources(0, 5, m_shaderResources);
#else
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, NormalGMap);
	glUniform1i(NormalGMapPosition, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, DiffuseGMap);
	glUniform1i(DiffuseGMapPosition, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, SpecularGMap);
	glUniform1i(SpecularGMapPosition, 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, DepthGMap);
	glUniform1i(DepthGMapPosition, 3);
#endif
}


//----------------------------------------------------------//


GodRayEffect::GodRayEffect(const std::wstring& filename)
	: Effect(std::wstring(L"FX/ScreenQuad_vs") + EXT, filename + L"_ps" + EXT)
{
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}

void GodRayEffect::UpdateConstantBuffer() {
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
	ID3D11Buffer* cbuf[] = { m_perFrameCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(1, 1, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(1, 1, cbuf);
#endif
}

void GodRayEffect::BindShaderResource() {
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
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

	CubeMapPosition = glGetUniformLocation(m_shaderProgram, "gCubeMap");
	assert(CubeMapPosition >= 0);
#endif
}

void SkyboxEffect::UpdateConstantBuffer() {
#ifdef GRAPHICS_D3D11
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

void SkyboxEffect::BindConstantBuffer() {
#ifdef GRAPHICS_D3D11
	ID3D11Buffer* cbuf[] = { m_perObjectCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 1, cbuf);
#endif
}

SkyboxEffect::~SkyboxEffect()
{
#ifdef GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}

void SkyboxEffect::BindShaderResource() {
#ifdef GRAPHICS_D3D11
	//static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShaderResources(0, 4, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShaderResources(0, 1, m_shaderResources);
#else
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap);
	//glUniform1i(CubeMapPosition, 0);
	glUniformHandleui64ARB(CubeMapPosition, CubeMap);
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
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
	ReleaseCOM(m_inputLayout);
#endif
}

void DebugLineEffect::UpdateConstantBuffer() {
#ifdef GRAPHICS_D3D11
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
#ifdef GRAPHICS_D3D11
	ID3D11Buffer* cbuf[] = { m_perObjectCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 1, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(0, 1, cbuf);
#endif
}

void DebugLineEffect::BindShaderResource() {

}


#ifdef GRAPHICS_D3D11
//----------------------------------------------------------//


VBlurEffect::VBlurEffect(const std::wstring& filename)
	: Effect(L"", L"", L"", L"", L"", filename + L"_cs" + EXT)
{
	m_shaderResources = new ID3D11ShaderResourceView*[1];
	for (int i = 0; i < 1; i++) {
		m_shaderResources[i] = 0;
	}

	m_outputShaderResources = new ID3D11ShaderResourceView*[2];
	m_unorderedAccessViews = new ID3D11UnorderedAccessView*[2];

	for (int i = 0; i < 2; i++) {

		D3D11_TEXTURE2D_DESC blurredTexDesc;
		blurredTexDesc.Width = Renderer::Instance()->GetScreenWidth();
		blurredTexDesc.Height = Renderer::Instance()->GetScreenHeight();
		blurredTexDesc.MipLevels = 1;
		blurredTexDesc.ArraySize = 1;
		blurredTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		blurredTexDesc.SampleDesc.Count = 1;
		blurredTexDesc.SampleDesc.Quality = 0;
		blurredTexDesc.Usage = D3D11_USAGE_DEFAULT;
		blurredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		blurredTexDesc.CPUAccessFlags = 0;
		blurredTexDesc.MiscFlags = 0;

		ID3D11Texture2D* blurredTex = 0;
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateTexture2D(&blurredTexDesc, 0, &blurredTex));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = blurredTexDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateShaderResourceView(blurredTex, &srvDesc, &m_outputShaderResources[i]));

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = blurredTexDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateUnorderedAccessView(blurredTex, &uavDesc, &m_unorderedAccessViews[i]));


		ReleaseCOM(blurredTex);

	}

	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(SETTING_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &m_settingConstantBuffer;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, &InitData,
			&m_settingCB);

		assert(SUCCEEDED(hr));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
}

VBlurEffect::~VBlurEffect()
{
	ReleaseCOM(m_inputLayout);
}

void VBlurEffect::UpdateConstantBuffer() {
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_settingCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_settingConstantBuffer, sizeof(SETTING_CONSTANT_BUFFER));
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_settingCB, NULL);
	}
}

void VBlurEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_settingCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetConstantBuffers(0, 1, cbuf);
}

void VBlurEffect::BindShaderResource(int idx) {
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetShaderResources(0, 1, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetUnorderedAccessViews(0, 1, &m_unorderedAccessViews[idx], 0);
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
}

DeferredGeometryTessPassEffect::~DeferredGeometryTessPassEffect()
{
	ReleaseCOM(m_inputLayout);
}

void DeferredGeometryTessPassEffect::UpdateConstantBuffer() {
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
}

void DeferredGeometryTessPassEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perObjectCB, m_perFrameCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 2, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(0, 2, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetConstantBuffers(0, 2, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetConstantBuffers(0, 2, cbuf);
}

void DeferredGeometryTessPassEffect::BindShaderResource() {
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShaderResources(0, 5, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShaderResources(0, 5, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetShaderResources(0, 5, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetShaderResources(0, 5, m_shaderResources);
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

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &m_perObjConstantBuffer;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		// Create the buffer.
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, &InitData, &m_perObjectCB));
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

		// Create the buffer.
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, &InitData, &m_perFrameCB));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
}

DeferredGeometryTerrainPassEffect::~DeferredGeometryTerrainPassEffect()
{
	ReleaseCOM(m_inputLayout);
}

void DeferredGeometryTerrainPassEffect::UpdateConstantBuffer() {
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
}

void DeferredGeometryTerrainPassEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perObjectCB, m_perFrameCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 2, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(0, 2, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetConstantBuffers(0, 2, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetConstantBuffers(0, 2, cbuf);
}

void DeferredGeometryTerrainPassEffect::BindShaderResource() {
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShaderResources(0, 8, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShaderResources(0, 8, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetShaderResources(0, 8, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetShaderResources(0, 8, m_shaderResources);
}


//----------------------------------------------------------//

TerrainShadowMapEffect::TerrainShadowMapEffect(const std::wstring& filename)
	: Effect(
	filename + L"_vs" + EXT,
	L"",
	L"",
	filename + L"_hs" + EXT,
	filename + L"_ds" + EXT)
{
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

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &m_perObjConstantBuffer;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		// Create the buffer.
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, &InitData, &m_perObjectCB));
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

		// Create the buffer.
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, &InitData, &m_perFrameCB));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
}

TerrainShadowMapEffect::~TerrainShadowMapEffect()
{
	ReleaseCOM(m_inputLayout);
}

void TerrainShadowMapEffect::UpdateConstantBuffer() {
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
}

void TerrainShadowMapEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perObjectCB, m_perFrameCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 2, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(0, 2, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetConstantBuffers(0, 2, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetConstantBuffers(0, 2, cbuf);
}

void TerrainShadowMapEffect::BindShaderResource() {
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShaderResources(0, 8, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShaderResources(0, 8, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetShaderResources(0, 8, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetShaderResources(0, 8, m_shaderResources);
}


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


HBlurEffect::HBlurEffect(const std::wstring& filename)
	: Effect(L"", L"", L"", L"", L"", filename + L"_cs" + EXT)
{
	m_shaderResources = new ID3D11ShaderResourceView*[1];
	for (int i = 0; i < 1; i++) {
		m_shaderResources[i] = 0;
	}

	m_outputShaderResources = new ID3D11ShaderResourceView*[2];
	m_unorderedAccessViews = new ID3D11UnorderedAccessView*[2];

	for (int i = 0; i < 2; i++) {
		D3D11_TEXTURE2D_DESC blurredTexDesc;
		blurredTexDesc.Width = Renderer::Instance()->GetScreenWidth();
		blurredTexDesc.Height = Renderer::Instance()->GetScreenHeight();
		blurredTexDesc.MipLevels = 1;
		blurredTexDesc.ArraySize = 1;
		blurredTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		blurredTexDesc.SampleDesc.Count = 1;
		blurredTexDesc.SampleDesc.Quality = 0;
		blurredTexDesc.Usage = D3D11_USAGE_DEFAULT;
		blurredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		blurredTexDesc.CPUAccessFlags = 0;
		blurredTexDesc.MiscFlags = 0;

		ID3D11Texture2D* blurredTex = 0;
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateTexture2D(&blurredTexDesc, 0, &blurredTex));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = blurredTexDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateShaderResourceView(blurredTex, &srvDesc, &m_outputShaderResources[i]));

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = blurredTexDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		
		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateUnorderedAccessView(blurredTex, &uavDesc, &m_unorderedAccessViews[i]));


		ReleaseCOM(blurredTex);

		{
			// Fill in a buffer description.
			D3D11_BUFFER_DESC cbDesc;
			cbDesc.ByteWidth = sizeof(SETTING_CONSTANT_BUFFER);
			cbDesc.Usage = D3D11_USAGE_DYNAMIC;
			cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbDesc.MiscFlags = 0;
			cbDesc.StructureByteStride = 0;

			// Fill in the subresource data.
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = &m_settingConstantBuffer;
			InitData.SysMemPitch = 0;
			InitData.SysMemSlicePitch = 0;

			HRESULT hr;
			// Create the buffer.
			hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, &InitData,
				&m_settingCB);

			assert(SUCCEEDED(hr));
		}
	}
	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
}

HBlurEffect::~HBlurEffect()
{
	ReleaseCOM(m_inputLayout);
}

void HBlurEffect::UpdateConstantBuffer() {
		{
			D3D11_MAPPED_SUBRESOURCE ms;
			static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_settingCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
			memcpy(ms.pData, &m_settingConstantBuffer, sizeof(SETTING_CONSTANT_BUFFER));
			static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_settingCB, NULL);
		}
}

void HBlurEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_settingCB };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetConstantBuffers(0, 1, cbuf);
}

void HBlurEffect::BindShaderResource(int idx) {
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetShaderResources(0, 1, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetUnorderedAccessViews(0, 1, &m_unorderedAccessViews[idx], 0);
}


//-------------------------------------------//


BlurEffect::BlurEffect(const std::wstring& filename)
	: Effect(std::wstring(L"FX/ScreenQuad_vs") + EXT, filename + L"_ps" + EXT)
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

	m_shaderResources = new ID3D11ShaderResourceView*[3];
	for (int i = 0; i < 3; i++) {
		m_shaderResources[i] = 0;
	}

	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(SETTING_CONSTANT_BUFFER);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &m_settingConstantBuffer;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		HRESULT hr;
		// Create the buffer.
		hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&cbDesc, &InitData,
			&m_settingCB);

		assert(SUCCEEDED(hr));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
}

BlurEffect::~BlurEffect()
{
	ReleaseCOM(m_inputLayout);
}

void BlurEffect::UpdateConstantBuffer() {
		{
			D3D11_MAPPED_SUBRESOURCE ms;
			static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_settingCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
			memcpy(ms.pData, &m_settingConstantBuffer, sizeof(SETTING_CONSTANT_BUFFER));
			static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_settingCB, NULL);
		}
}

void BlurEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_settingCB };
	//static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 1, cbuf);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(0, 1, cbuf);
}

void BlurEffect::BindShaderResource() {
	//	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShaderResources(0, 2, m_shaderResources);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShaderResources(0, 3, m_shaderResources);
}

#else


/********************************************************************/


bool Effect::ReadShaderFile(std::wstring filename, char* shaderContent, int maxLength) {
	std::string s(filename.begin(), filename.end());
	const char* lFilename = s.c_str();
	FILE* file = fopen(lFilename, "r");
	size_t current_len = 0;
	char line[2048];

	shaderContent[0] = '\0'; /* reset string */
	if (!file) {
		assert(false);
// 		OutputDebugStringA("ERROR: opening file for reading: %s\n", lFilename);
// 		return false;
	}
	strcpy(line, ""); /* remember to clean up before using for first time! */
	while (!feof(file)) {
		if (NULL != fgets(line, 2048, file)) {
			current_len += strlen(line); /* +1 for \n at end */
			if (current_len >= maxLength) {
				assert(false);
// 				OutputDebugStringA(
// 					"ERROR: shader length is longer than string buffer length %i\n",
// 					maxLength
// 					);
			}
			strcat(shaderContent, line);
		}
	}
	if (EOF == fclose(file)) { /* probably unnecesssary validation */
		assert(false);
// 		OutputDebugStringA("ERROR: closing file from reading %s\n", lFilename);
// 		return false;
	}
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

#ifdef PLATFORM_ANDROID

SimpleMeshEffect::SimpleMeshEffect(const std::string& vsPath, const std::string& psPath)
	:Effect(vsPath, psPath)
{

}

SimpleMeshEffect::SimpleMeshEffect(const std::string& filename)
	: Effect(filename + "_vs.glsl", filename + "_ps.glsl")
{
	glGenBuffers(1, &m_perFrameUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_perFrameUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PERFRAME_UNIFORM_BUFFER), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &m_perObjectUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_perObjectUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PEROBJ_UNIFORM_BUFFER), NULL, GL_DYNAMIC_DRAW);


	GLuint perFrameUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerFrame");
	//glUniformBlockBinding(m_shaderProgram, perFrameUBOPos, 1);

	GLint perObjUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerObj");
	//glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);

// 	DiffuseMapPosition = glGetUniformLocation(m_shaderProgram, "gDiffuseMap");
// 	NormalMapPosition = glGetUniformLocation(m_shaderProgram, "gNormalMap");
// 	ShadowMapPosition = glGetUniformLocation(m_shaderProgram, "gShadowMap");
// 	CubeMapPosition = glGetUniformLocation(m_shaderProgram, "gCubeMap");

}

SimpleMeshEffect::~SimpleMeshEffect()
{

}

void SimpleMeshEffect::UpdateConstantBuffer() {
	// 2nd para, should match layout binding in GLSL
	// if no binding is specified
	// call glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);
	// to specify the binding point and math 2nd param with it

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
}

void SimpleMeshEffect::BindConstantBuffer() {

}

// void SimpleMeshEffect::BindShaderResource() {
// 	glActiveTexture(GL_TEXTURE0);
// 	glBindTexture(GL_TEXTURE_2D, DiffuseMap);
// 	glUniform1i(DiffuseMapPosition, 0);
// 
// 	glActiveTexture(GL_TEXTURE1);
// 	glBindTexture(GL_TEXTURE_2D, NormalMap);
// 	glUniform1i(NormalMapPosition, 1);
// 
// 	glActiveTexture(GL_TEXTURE2);
// 	glBindTexture(GL_TEXTURE_2D, ShadowMapTex);
// 	glUniform1i(ShadowMapPosition, 2);
// 
// 	glActiveTexture(GL_TEXTURE3);
// 	glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapTex);
// 	glUniform1i(CubeMapPosition, 3);
// }

//*********************************************************************//

#endif


EffectsManager* EffectsManager::_instance = nullptr;
EffectsManager::EffectsManager() 
	: m_shadowMapEffect(nullptr)
	, m_deferredGeometryPassEffect(nullptr)
	, m_deferredShadingPassEffect(nullptr)
	, m_godrayEffect(nullptr)
	, m_skyboxEffect(nullptr)
	, m_blurEffect(nullptr)
	, m_vblurEffect(nullptr)
{

#ifndef PLATFORM_ANDROID

#ifdef GRAPHICS_D3D11
	m_shadowMapEffect = new ShadowMapEffect(L"FX/ShadowMap");
	m_terrainShadowMapEffect = new TerrainShadowMapEffect(L"FX/DeferredGeometryTerrainPass");
	m_deferredGeometryPassEffect = new DeferredGeometryPassEffect(L"FX/DeferredGeometryPass");
	m_deferredGeometryTessPassEffect = new DeferredGeometryTessPassEffect(L"FX/DeferredGeometryTessPass");
	m_deferredGeometryTerrainPassEffect = new DeferredGeometryTerrainPassEffect(L"FX/DeferredGeometryTerrainPass");
	m_deferredShadingPassEffect = new DeferredShadingPassEffect(L"FX/DeferredShadingPass");
	m_deferredShadingCSEffect = new DeferredShadingCS(L"FX/DeferredShading");

	m_blurEffect = new BlurEffect(L"FX/Blur");
	m_vblurEffect = new VBlurEffect(L"FX/VBlur");
	m_hblurEffect = new HBlurEffect(L"FX/HBlur");
	m_godrayEffect = new GodRayEffect(L"FX/GodRay");
	m_skyboxEffect = new SkyboxEffect(L"FX/Skybox");
	m_debugLineEffect = new DebugLineEffect(L"FX/DebugLine");

#else
	m_shadowMapEffect = new ShadowMapEffect(L"FX/ShadowMap");
	m_deferredGeometryPassEffect = new DeferredGeometryPassEffect(L"FX/DeferredGeometryPass");
	m_deferredShadingPassEffect = new DeferredShadingPassEffect(L"FX/DeferredShadingPass");

	m_skyboxEffect = new SkyboxEffect(L"FX/Skybox");
	m_debugLineEffect = new DebugLineEffect(L"FX/DebugLine");
	m_godrayEffect = new GodRayEffect(L"FX/GodRay");
#endif

#else
	m_debugLineEffect = new DebugLineEffect("Shaders/DebugLine");
	m_simpleMeshEffect = new SimpleMeshEffect("Shaders/SimpleMesh");
#endif
}

EffectsManager::~EffectsManager() {
	//for (int i = 0; i < m_effects.size(); i++) {
	//	delete m_effects[i];
	//}
	//SafeDelete(m_stdMeshEffect);
#ifndef PLATFORM_ANDROID
	SafeDelete(m_shadowMapEffect);
	SafeDelete(m_deferredGeometryPassEffect);
	SafeDelete(m_deferredShadingPassEffect);
	//SafeDelete(m_screenQuadEffect);
	SafeDelete(m_godrayEffect);
	SafeDelete(m_skyboxEffect);
	SafeDelete(m_blurEffect);
	SafeDelete(m_vblurEffect);
#endif
}