#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "D3DCompiler.h"
#include "imgui.h"

#pragma warning( disable : 4996 )

namespace StenGine
{

inline std::wstring GetExt()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		return L".hlsl";
	}
	case RenderBackend::OPENGL4:
	{
		return L".glsl";
	}
	}

	assert(false);
	return L".fx";
}

Effect::Effect(const std::wstring& filename)
{

}

Effect::Effect(const std::wstring& vsPath,
			   const std::wstring& psPath,
			   const std::wstring& gsPath = L"",
			   const std::wstring& hsPath = L"",
			   const std::wstring& dsPath = L"",
			   const std::wstring& csPath = L"")
			   : m_d3d11vertexShader(nullptr)
			   , m_d3d11pixelShader(nullptr)
			   , m_d3d11geometryShader(nullptr)
			   , m_d3d11hullShader(nullptr)
			   , m_d3d11domainShader(nullptr)
			   , m_d3d11computeShader(nullptr)
			   , m_d3d11inputLayout(0)
			   , m_glvertexShader(0)
			   , m_glpixelShader(0)
			   , m_glgeometryShader(0)
			   , m_glhullShader(0)
			   , m_gldomainShader(0)
			   , m_glcomputeShader(0)
			   , m_glinputLayout(0)
			   , m_vsBlob(nullptr)
			   , m_psBlob(nullptr)
			   , m_gsBlob(nullptr)
			   , m_hsBlob(nullptr)
			   , m_dsBlob(nullptr)
			   , m_csBlob(nullptr)
			   , m_shaderResources(nullptr)

{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		// Add error checking
		HRESULT hr;

		if (vsPath.length()) {
			ReadShaderFile(vsPath, &m_vsBlob, "vs_5_0");
			hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateVertexShader(
				m_vsBlob->GetBufferPointer(),
				m_vsBlob->GetBufferSize(),
				nullptr,
				&m_d3d11vertexShader
			);
			assert(SUCCEEDED(hr));
		}

		if (psPath.length()) {
			ReadShaderFile(psPath, &m_psBlob, "ps_5_0");
			hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreatePixelShader(
				m_psBlob->GetBufferPointer(),
				m_psBlob->GetBufferSize(),
				nullptr,
				&m_d3d11pixelShader
			);
			assert(SUCCEEDED(hr));
		}

		if (gsPath.length()) {
			ReadShaderFile(gsPath, &m_gsBlob, "gs_5_0");
			hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateGeometryShader(
				m_gsBlob->GetBufferPointer(),
				m_gsBlob->GetBufferSize(),
				nullptr,
				&m_d3d11geometryShader
			);
			assert(SUCCEEDED(hr));
		}

		if (hsPath.length()) {
			ReadShaderFile(hsPath, &m_hsBlob, "hs_5_0");
			hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateHullShader(
				m_hsBlob->GetBufferPointer(),
				m_hsBlob->GetBufferSize(),
				nullptr,
				&m_d3d11hullShader
			);
			assert(SUCCEEDED(hr));
		}

		if (dsPath.length()) {
			ReadShaderFile(dsPath, &m_dsBlob, "ds_5_0");
			hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateDomainShader(
				m_dsBlob->GetBufferPointer(),
				m_dsBlob->GetBufferSize(),
				nullptr,
				&m_d3d11domainShader
			);
			assert(SUCCEEDED(hr));
		}

		if (csPath.length()) {
			ReadShaderFile(csPath, &m_csBlob, "cs_5_0");
			hr = static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateComputeShader(
				m_csBlob->GetBufferPointer(),
				m_csBlob->GetBufferSize(),
				nullptr,
				&m_d3d11computeShader
			);
			assert(SUCCEEDED(hr));
		}
		break;
	}
	case RenderBackend::OPENGL4:
	{
		char shaderbuffer[1024 * 256];
		const GLchar* p;
		int params = -1;

		if (vsPath.length()) {
			(ReadShaderFile(vsPath, shaderbuffer, 1024 * 256));
			m_glvertexShader = glCreateShader(GL_VERTEX_SHADER);

			p = (const GLchar*)shaderbuffer;
			glShaderSource(m_glvertexShader, 1, &p, NULL);
			glCompileShader(m_glvertexShader);
			/* check for shader compile errors - very important! */

			glGetShaderiv(m_glvertexShader, GL_COMPILE_STATUS, &params);
			if (GL_TRUE != params) {
				GLint maxLength = 0;
				glGetShaderiv(m_glvertexShader, GL_INFO_LOG_LENGTH, &maxLength);

				//The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(m_glvertexShader, maxLength, &maxLength, &infoLog[0]);
				//We don't need the shader anymore.
				glDeleteShader(m_glvertexShader);

				OutputDebugStringA(&infoLog[0]);
				assert(false);
				return;

				// 			fprintf(stderr, "ERROR: GL shader index %i did not compile\n", m_vertexShader);
				// 			_print_shader_info_log(vs);
			}
		}

		if (hsPath.length()) {
			(ReadShaderFile(hsPath, shaderbuffer, 1024 * 256));
			m_glhullShader = glCreateShader(GL_TESS_CONTROL_SHADER);
			p = (const GLchar*)shaderbuffer;
			glShaderSource(m_glhullShader, 1, &p, NULL);
			glCompileShader(m_glhullShader);

			/* check for shader compile errors - very important! */

			glGetShaderiv(m_glhullShader, GL_COMPILE_STATUS, &params);
			if (GL_TRUE != params) {
				GLint maxLength = 0;
				glGetShaderiv(m_glhullShader, GL_INFO_LOG_LENGTH, &maxLength);

				//The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(m_glhullShader, maxLength, &maxLength, &infoLog[0]);
				//We don't need the shader anymore.
				glDeleteShader(m_glhullShader);

				OutputDebugStringA(&infoLog[0]);
				assert(false);
				return;

				// 			fprintf(stderr, "ERROR: GL shader index %i did not compile\n", m_vertexShader);
				// 			_print_shader_info_log(vs);
			}
		}

		if (dsPath.length()) {
			(ReadShaderFile(dsPath, shaderbuffer, 1024 * 256));
			m_gldomainShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
			p = (const GLchar*)shaderbuffer;
			glShaderSource(m_gldomainShader, 1, &p, NULL);
			glCompileShader(m_gldomainShader);

			/* check for shader compile errors - very important! */

			glGetShaderiv(m_gldomainShader, GL_COMPILE_STATUS, &params);
			if (GL_TRUE != params) {
				GLint maxLength = 0;
				glGetShaderiv(m_gldomainShader, GL_INFO_LOG_LENGTH, &maxLength);

				//The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(m_gldomainShader, maxLength, &maxLength, &infoLog[0]);
				//We don't need the shader anymore.
				glDeleteShader(m_gldomainShader);

				OutputDebugStringA(&infoLog[0]);
				assert(false);
				return;

				// 			fprintf(stderr, "ERROR: GL shader index %i did not compile\n", m_vertexShader);
				// 			_print_shader_info_log(vs);
			}
		}

		if (psPath.length()) {
			(ReadShaderFile(psPath, shaderbuffer, 1024 * 256));
			m_glpixelShader = glCreateShader(GL_FRAGMENT_SHADER);
			p = (const GLchar*)shaderbuffer;
			glShaderSource(m_glpixelShader, 1, &p, NULL);
			glCompileShader(m_glpixelShader);
			/* check for shader compile errors - very important! */

			glGetShaderiv(m_glpixelShader, GL_COMPILE_STATUS, &params);
			if (GL_TRUE != params) {
				//assert(false);
				// 			fprintf(stderr, "ERROR: GL shader index %i did not compile\n", m_vertexShader);
				// 			_print_shader_info_log(vs);

				GLint maxLength = 0;
				glGetShaderiv(m_glpixelShader, GL_INFO_LOG_LENGTH, &maxLength);

				//The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(m_glpixelShader, maxLength, &maxLength, &infoLog[0]);
				//We don't need the shader anymore.
				glDeleteShader(m_glpixelShader);

				OutputDebugStringA(&infoLog[0]);
				assert(false);
				return;
			}
		}

		if (csPath.length()) {
			(ReadShaderFile(csPath, shaderbuffer, 1024 * 256));
			m_glcomputeShader = glCreateShader(GL_COMPUTE_SHADER);
			p = (const GLchar*)shaderbuffer;
			glShaderSource(m_glcomputeShader, 1, &p, NULL);
			glCompileShader(m_glcomputeShader);

			/* check for shader compile errors - very important! */

			glGetShaderiv(m_glcomputeShader, GL_COMPILE_STATUS, &params);
			if (GL_TRUE != params) {
				//assert(false);
				// 			fprintf(stderr, "ERROR: GL shader index %i did not compile\n", m_vertexShader);
				// 			_print_shader_info_log(vs);

				GLint maxLength = 0;
				glGetShaderiv(m_glcomputeShader, GL_INFO_LOG_LENGTH, &maxLength);

				//The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(m_glcomputeShader, maxLength, &maxLength, &infoLog[0]);
				//We don't need the shader anymore.
				glDeleteShader(m_glcomputeShader);

				OutputDebugStringA(&infoLog[0]);
				assert(false);
				return;
			}
		}

		m_shaderProgram = glCreateProgram();
		if (vsPath.length()) glAttachShader(m_shaderProgram, m_glvertexShader);
		if (hsPath.length()) glAttachShader(m_shaderProgram, m_glhullShader);
		if (dsPath.length()) glAttachShader(m_shaderProgram, m_gldomainShader);
		if (psPath.length()) glAttachShader(m_shaderProgram, m_glpixelShader);
		if (csPath.length()) glAttachShader(m_shaderProgram, m_glcomputeShader);
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
		break;
	}
	}
}

Effect::~Effect()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		SafeDeleteArray(m_shaderResources);
		ReleaseCOM(m_d3d11vertexShader);
		ReleaseCOM(m_d3d11pixelShader);
		ReleaseCOM(m_d3d11geometryShader);
		ReleaseCOM(m_d3d11hullShader);
		ReleaseCOM(m_d3d11domainShader);
		ReleaseCOM(m_d3d11computeShader);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		glDeleteShader(m_glvertexShader);
		glDeleteShader(m_glpixelShader);
		glDeleteShader(m_glgeometryShader);
		glDeleteShader(m_glhullShader);
		glDeleteShader(m_gldomainShader);
		glDeleteShader(m_glcomputeShader);
		glDeleteProgram(m_shaderProgram);
		break;
	}
	}
}

void Effect::UnBindConstantBuffer() {
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetConstantBuffers(0, 0, nullptr);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetConstantBuffers(0, 0, nullptr);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetConstantBuffers(0, 0, nullptr);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetConstantBuffers(0, 0, nullptr);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->GSSetConstantBuffers(0, 0, nullptr);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetConstantBuffers(0, 0, nullptr);
}

void Effect::UnBindShaderResource() {
	static ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShaderResources(0, 16, nullSRV);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShaderResources(0, 16, nullSRV);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetShaderResources(0, 16, nullSRV);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetShaderResources(0, 16, nullSRV);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->GSSetShaderResources(0, 16, nullSRV);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetShaderResources(0, 16, nullSRV);
}


void Effect::SetShader() {
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		//if (m_vertexShader)
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShader(m_d3d11vertexShader, 0, 0);
		//if (m_pixelShader)											
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShader(m_d3d11pixelShader, 0, 0);
		//if (m_geometryShader)																
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->GSSetShader(m_d3d11geometryShader, 0, 0);
		//if (m_hullShader)																	
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetShader(m_d3d11hullShader, 0, 0);
		//if (m_domainShader)														
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetShader(m_d3d11domainShader, 0, 0);
		//if (m_domainShader)														
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetShader(m_d3d11computeShader, 0, 0);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		// gl
		glUseProgram(m_shaderProgram);
		break;
	}
	}
}

void Effect::UnSetShader() {
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->VSSetShader(nullptr, 0, 0);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->PSSetShader(nullptr, 0, 0);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->HSSetShader(nullptr, 0, 0);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->GSSetShader(nullptr, 0, 0);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->DSSetShader(nullptr, 0, 0);
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetShader(nullptr, 0, 0);
}

void* Effect::GetInputLayout()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
		return (void*)m_d3d11inputLayout;
	case RenderBackend::OPENGL4:
		return (void*)m_glinputLayout;
	}
	return nullptr;
}

void Effect::UnbindUnorderedAccessViews() {
	static ID3D11UnorderedAccessView* nullUAV[7] = { 0 };
	static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->CSSetUnorderedAccessViews(0, 7, nullUAV, 0);
}

void Effect::ReadShaderFile(std::wstring filename, ID3DBlob **blob, char* target, char* entryPoint) {
	HRESULT hr;

	wprintf(L"%s\n", filename.c_str());

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

	assert(SUCCEEDED(hr));
}

void Effect::SetShaderResources(ID3D11ShaderResourceView* res, int idx) {
	m_shaderResources[idx] = res;
}

ID3D11ShaderResourceView* Effect::GetOutputShaderResource(int idx) {
	return m_outputShaderResources[idx];
}

//----------------------------------------------------------//


ShadowMapEffect::ShadowMapEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + GetExt(), (Renderer::GetRenderBackend() == RenderBackend::D3D11) ? L"" : (std::wstring(L"FX/ZOnly_ps.glsl")))
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(), &m_d3d11inputLayout));


		ReleaseCOM(m_vsBlob);
		ReleaseCOM(m_psBlob);
		ReleaseCOM(m_gsBlob);
		ReleaseCOM(m_hsBlob);
		ReleaseCOM(m_dsBlob);
		ReleaseCOM(m_csBlob);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		glCreateVertexArrays(1, &m_glinputLayout);
		glEnableVertexArrayAttrib(m_glinputLayout, 0);
		glVertexArrayAttribFormat(m_glinputLayout, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex::StdMeshVertex, Pos));
		glVertexArrayAttribBinding(m_glinputLayout, 0, 0);

		GLint perObjUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerObj");
		glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);

		break;
	}
	}

	m_perObjectCB = new GPUBuffer(sizeof(PEROBJ_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
}

ShadowMapEffect::~ShadowMapEffect()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
		ReleaseCOM(m_d3d11inputLayout);
		break;
	case RenderBackend::OPENGL4:
		glDeleteVertexArrays(1, &m_glinputLayout);
		break;
	}

	SafeDelete(m_perObjectCB);
}


//------------------------------------------------------------//

DeferredGeometryPassEffect::DeferredGeometryPassEffect(const std::wstring& vsPath, const std::wstring& psPath, const std::wstring& gsPath, const std::wstring& hsPath, const std::wstring& dsPath)
	:Effect(vsPath, psPath, gsPath, hsPath, dsPath)
{

}

DeferredGeometryPassEffect::DeferredGeometryPassEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + GetExt(), filename + L"_ps" + GetExt())
{
	PrepareBuffer();
}

DeferredGeometryPassEffect::~DeferredGeometryPassEffect()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
		ReleaseCOM(m_d3d11inputLayout);
		break;
	case RenderBackend::OPENGL4:
		glDeleteVertexArrays(1, &m_glinputLayout);
		SafeDelete(m_textureCB);
		break;
	}

	SafeDelete(m_perObjectCB);
	SafeDelete(m_perFrameCB);
	
}

void DeferredGeometryPassEffect::PrepareBuffer()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 4, m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(), &m_d3d11inputLayout));

		m_shaderResources = new ID3D11ShaderResourceView*[5];

		for (int i = 0; i < 5; i++) {
			m_shaderResources[i] = 0;
		}

		ReleaseCOM(m_vsBlob);
		ReleaseCOM(m_psBlob);
		ReleaseCOM(m_gsBlob);
		ReleaseCOM(m_hsBlob);
		ReleaseCOM(m_dsBlob);
		ReleaseCOM(m_csBlob);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		glCreateVertexArrays(1, &m_glinputLayout);

		glEnableVertexArrayAttrib(m_glinputLayout, 0);
		glEnableVertexArrayAttrib(m_glinputLayout, 1);
		glEnableVertexArrayAttrib(m_glinputLayout, 2);
		glEnableVertexArrayAttrib(m_glinputLayout, 3);

		glVertexArrayAttribFormat(m_glinputLayout, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex::StdMeshVertex, Pos));
		glVertexArrayAttribFormat(m_glinputLayout, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex::StdMeshVertex, Normal));
		glVertexArrayAttribFormat(m_glinputLayout, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex::StdMeshVertex, Tangent));
		glVertexArrayAttribFormat(m_glinputLayout, 3, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex::StdMeshVertex, TexUV));

		glVertexArrayAttribBinding(m_glinputLayout, 0, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 1, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 2, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 3, 0);

		GLint perFrameUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerFrame");
		glUniformBlockBinding(m_shaderProgram, perFrameUBOPos, 1);

		GLint perObjUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerObj");
		glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);

		GLint texUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubTextures");
		glUniformBlockBinding(m_shaderProgram, texUBOPos, 2);

		m_textureCB = new GPUBuffer(sizeof(BINDLESS_TEXTURE_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);

		break;
	}
	}

	m_perFrameCB = new GPUBuffer(sizeof(PERFRAME_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
	m_perObjectCB = new GPUBuffer(sizeof(PEROBJ_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
}

//------------------------------------------------------------//

DeferredSkinnedGeometryPassEffect::DeferredSkinnedGeometryPassEffect(const std::wstring& vsPath, const std::wstring& psPath, const std::wstring& gsPath, const std::wstring& hsPath, const std::wstring& dsPath)
	:Effect(vsPath, psPath, gsPath, hsPath, dsPath)
{

}

DeferredSkinnedGeometryPassEffect::DeferredSkinnedGeometryPassEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + GetExt(), filename + L"_ps" + GetExt())
{
	PrepareBuffer();
}

DeferredSkinnedGeometryPassEffect::~DeferredSkinnedGeometryPassEffect()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
		ReleaseCOM(m_d3d11inputLayout);
		break;
	case RenderBackend::OPENGL4:
		glDeleteVertexArrays(1, &m_glinputLayout);
		SafeDelete(m_textureCB);
		break;
	}

	SafeDelete(m_perFrameCB);
	SafeDelete(m_perObjectCB);
	SafeDelete(m_matrixPaletteSB);
}

void DeferredSkinnedGeometryPassEffect::PrepareBuffer()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex::SkinnedMeshVertex, Pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex::SkinnedMeshVertex, Normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex::SkinnedMeshVertex, Tangent), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex::SkinnedMeshVertex, TexUV), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "JOINTWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex::SkinnedMeshVertex, JointWeights), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "JOINTINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(Vertex::SkinnedMeshVertex, JointIndices), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 6, m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(), &m_d3d11inputLayout));

		ReleaseCOM(m_vsBlob);
		ReleaseCOM(m_psBlob);
		ReleaseCOM(m_gsBlob);
		ReleaseCOM(m_hsBlob);
		ReleaseCOM(m_dsBlob);
		ReleaseCOM(m_csBlob);

		m_matrixPaletteSB = new GPUBuffer(sizeof(Mat4) * 64, BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);

		break;
	}
	case RenderBackend::OPENGL4:
	{
		glCreateVertexArrays(1, &m_glinputLayout);

		glEnableVertexArrayAttrib(m_glinputLayout, 0);
		glEnableVertexArrayAttrib(m_glinputLayout, 1);
		glEnableVertexArrayAttrib(m_glinputLayout, 2);
		glEnableVertexArrayAttrib(m_glinputLayout, 3);
		glEnableVertexArrayAttrib(m_glinputLayout, 4);
		glEnableVertexArrayAttrib(m_glinputLayout, 5);

		glVertexArrayAttribFormat(m_glinputLayout, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex::SkinnedMeshVertex, Pos));
		glVertexArrayAttribFormat(m_glinputLayout, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex::SkinnedMeshVertex, Normal));
		glVertexArrayAttribFormat(m_glinputLayout, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex::SkinnedMeshVertex, Tangent));
		glVertexArrayAttribFormat(m_glinputLayout, 3, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex::SkinnedMeshVertex, TexUV));
		glVertexArrayAttribFormat(m_glinputLayout, 4, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex::SkinnedMeshVertex, JointWeights));
		glVertexArrayAttribIFormat(m_glinputLayout, 5, 4, GL_UNSIGNED_INT, offsetof(Vertex::SkinnedMeshVertex, JointIndices));

		glVertexArrayAttribBinding(m_glinputLayout, 0, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 1, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 2, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 3, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 4, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 5, 0);

		GLint perFrameUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerFrame");
		glUniformBlockBinding(m_shaderProgram, perFrameUBOPos, 1);

		GLint perObjUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerObj");
		glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);

		GLint textureUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubTextures");
		glUniformBlockBinding(m_shaderProgram, textureUBOPos, 2);

		m_textureCB = new GPUBuffer(sizeof(BINDLESS_TEXTURE_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
		m_matrixPaletteSB = new GPUBuffer(sizeof(Mat4) * 64, BufferUsage::WRITE, nullptr, BufferType::SSBO);

		break;
	}
	}

	m_perFrameCB = new GPUBuffer(sizeof(PERFRAME_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
	m_perObjectCB = new GPUBuffer(sizeof(PEROBJ_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
}


//------------------------------------------------------------//


DeferredShadingPassEffect::DeferredShadingPassEffect(const std::wstring& filename)
	: Effect(std::wstring(L"../StenGine/FX/ScreenQuad_vs") + GetExt(), filename + L"_ps" + GetExt())
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(), &m_d3d11inputLayout));

		m_shaderResources = new ID3D11ShaderResourceView*[5];
		for (int i = 0; i < 5; i++) {
			m_shaderResources[i] = 0;
		}

		ReleaseCOM(m_vsBlob);
		ReleaseCOM(m_psBlob);
		ReleaseCOM(m_gsBlob);
		ReleaseCOM(m_hsBlob);
		ReleaseCOM(m_dsBlob);
		ReleaseCOM(m_csBlob);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		GLint perFrameUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerFrame");
		glUniformBlockBinding(m_shaderProgram, perFrameUBOPos, 0);

		GLint textureUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubTextures");
		glUniformBlockBinding(m_shaderProgram, textureUBOPos, 1);

		m_textureCB = new GPUBuffer(sizeof(BINDLESS_TEXTURE_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
		break;
	}
	}

	m_perFrameCB = new GPUBuffer(sizeof(PERFRAME_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
}

DeferredShadingPassEffect::~DeferredShadingPassEffect()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
		ReleaseCOM(m_d3d11inputLayout);
		break;
	case RenderBackend::OPENGL4:
		SafeDelete(m_textureCB);
		break;
	}

	SafeDelete(m_perFrameCB);
}

SkyboxEffect::SkyboxEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + GetExt(), filename + L"_ps" + GetExt())
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		m_shaderResources = new ID3D11ShaderResourceView*[1];
		for (int i = 0; i < 1; i++) {
			m_shaderResources[i] = 0;
		}

		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(), &m_d3d11inputLayout));

		ReleaseCOM(m_vsBlob);
		ReleaseCOM(m_psBlob);
		ReleaseCOM(m_gsBlob);
		ReleaseCOM(m_hsBlob);
		ReleaseCOM(m_dsBlob);
		ReleaseCOM(m_csBlob);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		GLint perObjUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerObj");
		glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);

		GLint textureUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubTextures");
		glUniformBlockBinding(m_shaderProgram, textureUBOPos, 1);

		m_textureCB = new GPUBuffer(sizeof(BINDLESS_TEXTURE_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
		break;
	}
	}

	m_perObjectCB = new GPUBuffer(sizeof(PEROBJ_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
}

SkyboxEffect::~SkyboxEffect()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		ReleaseCOM(m_d3d11inputLayout);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		SafeDelete(m_textureCB);
		break;
	}
	}

	SafeDelete(m_perObjectCB);
}

//------------------------------------------------------------//

DebugLineEffect::DebugLineEffect(const std::wstring& vsPath, const std::wstring& psPath, const std::wstring& gsPath, const std::wstring& hsPath, const std::wstring& dsPath)
	:Effect(vsPath, psPath, gsPath, hsPath, dsPath)
{

}

DebugLineEffect::DebugLineEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + GetExt(), filename + L"_ps" + GetExt())
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(), &m_d3d11inputLayout));

		ReleaseCOM(m_vsBlob);
		ReleaseCOM(m_psBlob);
		ReleaseCOM(m_gsBlob);
		ReleaseCOM(m_hsBlob);
		ReleaseCOM(m_dsBlob);
		ReleaseCOM(m_csBlob);
		break;
	}
	}

	m_perObjectCB = new GPUBuffer(sizeof(PEROBJ_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
}

DebugLineEffect::~DebugLineEffect()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		ReleaseCOM(m_d3d11inputLayout);
		break;
	}
	}

	SafeDelete(m_perObjectCB);
}


//----------------------------------------------------------//

DeferredGeometryTessPassEffect::DeferredGeometryTessPassEffect(const std::wstring& filename)
	: DeferredGeometryPassEffect(
		filename + L"_vs" + GetExt(),
		filename + L"_ps" + GetExt(),
		L"",
		filename + L"_hs" + GetExt(),
		filename + L"_ds" + GetExt())
{
	PrepareBuffer();
}

DeferredGeometryTessPassEffect::~DeferredGeometryTessPassEffect()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		ReleaseCOM(m_d3d11inputLayout);
		break;
	}
	}
}

//----------------------------------------------------------//

DeferredGeometryTerrainPassEffect::DeferredGeometryTerrainPassEffect(const std::wstring& filename)
	: Effect(
		filename + L"_vs" + GetExt(),
		filename + L"_ps" + GetExt(),
		L"",
		filename + L"_hs" + GetExt(),
		filename + L"_ds" + GetExt())
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		HRESULT hr = (static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 3, m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(), &m_d3d11inputLayout));

		m_shaderResources = new ID3D11ShaderResourceView*[8];

		for (int i = 0; i < 8; i++) {
			m_shaderResources[i] = 0;
		}

		ReleaseCOM(m_vsBlob);
		ReleaseCOM(m_psBlob);
		ReleaseCOM(m_gsBlob);
		ReleaseCOM(m_hsBlob);
		ReleaseCOM(m_dsBlob);
		ReleaseCOM(m_csBlob);

		break;
	}
	case RenderBackend::OPENGL4:
	{
		glCreateVertexArrays(1, &m_glinputLayout);

		glEnableVertexArrayAttrib(m_glinputLayout, 0);
		glEnableVertexArrayAttrib(m_glinputLayout, 1);
		glEnableVertexArrayAttrib(m_glinputLayout, 2);

		glVertexArrayAttribFormat(m_glinputLayout, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_glinputLayout, 1, 2, GL_FLOAT, GL_FALSE, 12);
		glVertexArrayAttribFormat(m_glinputLayout, 2, 2, GL_FLOAT, GL_FALSE, 20);

		glVertexArrayAttribBinding(m_glinputLayout, 0, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 1, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 2, 0);

		GLint perFrameUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerFrame");
		glUniformBlockBinding(m_shaderProgram, perFrameUBOPos, 1);

		GLint perObjUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerObj");
		glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);

		GLint textureUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubTextures");
		glUniformBlockBinding(m_shaderProgram, textureUBOPos, 2);

		m_textureCB = new GPUBuffer(sizeof(BINDLESS_TEXTURE_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);

		break;
	}
	}

	m_perFrameCB = new GPUBuffer(sizeof(PERFRAME_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
	m_perObjectCB = new GPUBuffer(sizeof(PEROBJ_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);

}

DeferredGeometryTerrainPassEffect::~DeferredGeometryTerrainPassEffect()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		ReleaseCOM(m_d3d11inputLayout);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		glDeleteVertexArrays(1, &m_glinputLayout);
		SafeDelete(m_textureCB);
		break;
	}
	}

	SafeDelete(m_perFrameCB);
	SafeDelete(m_perObjectCB);
}


TerrainShadowMapEffect::TerrainShadowMapEffect(const std::wstring& filename)
	: Effect(
		filename + L"_vs" + GetExt(),
		L"",
		L"",
		filename + L"_hs" + GetExt(),
		filename + L"_ds" + GetExt())
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		HRESULT hr = (static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 3, m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(), &m_d3d11inputLayout));

		m_shaderResources = new ID3D11ShaderResourceView*[8];

		for (int i = 0; i < 8; i++) {
			m_shaderResources[i] = 0;
		}

		ReleaseCOM(m_vsBlob);
		ReleaseCOM(m_psBlob);
		ReleaseCOM(m_gsBlob);
		ReleaseCOM(m_hsBlob);
		ReleaseCOM(m_dsBlob);
		ReleaseCOM(m_csBlob);
		break;
	}
	case RenderBackend::OPENGL4:
	{

		glCreateVertexArrays(1, &m_glinputLayout);

		glEnableVertexArrayAttrib(m_glinputLayout, 0);
		glEnableVertexArrayAttrib(m_glinputLayout, 1);
		glEnableVertexArrayAttrib(m_glinputLayout, 2);

		glVertexArrayAttribFormat(m_glinputLayout, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_glinputLayout, 1, 2, GL_FLOAT, GL_FALSE, 12);
		glVertexArrayAttribFormat(m_glinputLayout, 2, 2, GL_FLOAT, GL_FALSE, 20);

		glVertexArrayAttribBinding(m_glinputLayout, 0, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 1, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 2, 0);

		GLint perFrameUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerFrame");
		glUniformBlockBinding(m_shaderProgram, perFrameUBOPos, 1);

		GLint perObjUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubPerObj");
		glUniformBlockBinding(m_shaderProgram, perObjUBOPos, 0);

		GLint textureUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubTextures");
		glUniformBlockBinding(m_shaderProgram, textureUBOPos, 2);

		m_textureCB = new GPUBuffer(sizeof(BINDLESS_TEXTURE_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);

		break;
	}
	}

	m_perFrameCB = new GPUBuffer(sizeof(PERFRAME_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
	m_perObjectCB = new GPUBuffer(sizeof(PEROBJ_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);

}

TerrainShadowMapEffect::~TerrainShadowMapEffect()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		ReleaseCOM(m_d3d11inputLayout);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		SafeDelete(m_textureCB);
		glDeleteVertexArrays(1, &m_glinputLayout);
		break;
	}
	}

	SafeDelete(m_perObjectCB);
	SafeDelete(m_perFrameCB);
}

//----------------------------------------------------------//

VBlurEffect::VBlurEffect(const std::wstring& filename)
	: Effect(L"", L"", L"", L"", L"", filename + L"_cs" + GetExt())
{

}

//----------------------------------------------------------//

HBlurEffect::HBlurEffect(const std::wstring& filename)
	: Effect(L"", L"", L"", L"", L"", filename + L"_cs" + GetExt())
{

}

//-------------------------------------------//

ImGuiEffect::ImGuiEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + GetExt(), filename + L"_ps" + GetExt(), L"", L"", L"", L"")
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_INPUT_ELEMENT_DESC localLayout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->uv),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (size_t)(&((ImDrawVert*)0)->col), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(localLayout, 3, m_vsBlob->GetBufferPointer(), m_vsBlob->GetBufferSize(), &m_d3d11inputLayout));

		break;
	}
	case RenderBackend::OPENGL4:
	{
		glCreateVertexArrays(1, &m_glinputLayout);

		glEnableVertexArrayAttrib(m_glinputLayout, 0);
		glEnableVertexArrayAttrib(m_glinputLayout, 1);
		glEnableVertexArrayAttrib(m_glinputLayout, 2);

		glVertexArrayAttribFormat(m_glinputLayout, 0, 2, GL_FLOAT, GL_FALSE, offsetof(ImDrawVert, pos));
		glVertexArrayAttribFormat(m_glinputLayout, 1, 2, GL_FLOAT, GL_FALSE, offsetof(ImDrawVert, uv));
		glVertexArrayAttribFormat(m_glinputLayout, 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(ImDrawVert, col));

		glVertexArrayAttribBinding(m_glinputLayout, 0, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 1, 0);
		glVertexArrayAttribBinding(m_glinputLayout, 2, 0);

		GLint imguiUBOPos = glGetUniformBlockIndex(m_shaderProgram, "imGuiCB");
		glUniformBlockBinding(m_shaderProgram, imguiUBOPos, 0);

		GLint textureUBOPos = glGetUniformBlockIndex(m_shaderProgram, "ubTextures");
		glUniformBlockBinding(m_shaderProgram, textureUBOPos, 1);

		m_textureCB = new GPUBuffer(sizeof(BINDLESS_TEXTURE_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);

		break;
	}
	}

	m_imguiCB = new GPUBuffer(sizeof(IMGUI_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);

}

ImGuiEffect::~ImGuiEffect()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		ReleaseCOM(m_d3d11inputLayout);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		SafeDelete(m_textureCB);
		glDeleteVertexArrays(1, &m_glinputLayout);
		break;
	}
	}

	SafeDelete(m_imguiCB);
}

//-------------------------------------------//


BlurEffect::BlurEffect(const std::wstring& filename)
	: Effect(std::wstring(L"../StenGine/FX/ScreenQuad_vs") + GetExt(), filename + L"_ps" + GetExt())
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		HR(static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(), &m_d3d11inputLayout));

		ReleaseCOM(m_vsBlob);
		ReleaseCOM(m_psBlob);
		ReleaseCOM(m_gsBlob);
		ReleaseCOM(m_hsBlob);
		ReleaseCOM(m_dsBlob);
		ReleaseCOM(m_csBlob);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		GLint settingCBPos = glGetUniformBlockIndex(m_shaderProgram, "cbSettings");
		glUniformBlockBinding(m_shaderProgram, settingCBPos, 0);

		GLint texturesCBPos = glGetUniformBlockIndex(m_shaderProgram, "cbTextures");
		glUniformBlockBinding(m_shaderProgram, texturesCBPos, 1);

		m_textureCB = new GPUBuffer(sizeof(BINDLESS_TEXTURE_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);

		break;
	}
	}

	m_settingCB = new GPUBuffer(sizeof(SETTING_CONSTANT_BUFFER), BufferUsage::WRITE, nullptr, BufferType::CONSTANT_BUFFER);
}

BlurEffect::~BlurEffect()
{
	switch (Renderer::GetRenderBackend())
	{
	case RenderBackend::D3D11:
	{
		ReleaseCOM(m_d3d11inputLayout);
		break;
	}
	case RenderBackend::OPENGL4:
	{
		SafeDelete(m_textureCB);
		break;
	}
	}

	SafeDelete(m_settingCB);
}

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


//----------------------------------------------------------//

DEFINE_SINGLETON_CLASS(EffectsManager)

EffectsManager::EffectsManager()
	: m_shadowMapEffect(nullptr)
	, m_terrainShadowMapEffect(nullptr)
	, m_deferredGeometryPassEffect(nullptr)
	, m_deferredSkinnedGeometryPassEffect(nullptr)
	, m_deferredGeometryTerrainPassEffect(nullptr)
	, m_deferredGeometryTessPassEffect(nullptr)
	, m_deferredShadingPassEffect(nullptr)
	, m_skyboxEffect(nullptr)
	, m_blurEffect(nullptr)
	, m_vblurEffect(nullptr)
	, m_hblurEffect(nullptr)
	, m_debugLineEffect(nullptr)
{

	// TODO, add a function to get FX directory

	m_shadowMapEffect = std::make_unique<ShadowMapEffect>(L"../StenGine/FX/ShadowMap");
	m_deferredGeometryPassEffect = std::make_unique<DeferredGeometryPassEffect>(L"../StenGine/FX/DeferredGeometryPass");
	m_deferredSkinnedGeometryPassEffect = std::make_unique<DeferredSkinnedGeometryPassEffect>(L"../StenGine/FX/DeferredSkinnedGeometryPass");
	m_deferredShadingPassEffect = std::make_unique<DeferredShadingPassEffect>(L"../StenGine/FX/DeferredShadingPass");
	m_deferredGeometryTessPassEffect = std::make_unique<DeferredGeometryTessPassEffect>(L"../StenGine/FX/DeferredGeometryTessPass");
	m_skyboxEffect = std::make_unique<SkyboxEffect>(L"../StenGine/FX/Skybox");
	m_debugLineEffect = std::make_unique<DebugLineEffect>(L"../StenGine/FX/DebugLine");
	m_deferredGeometryTerrainPassEffect = std::make_unique<DeferredGeometryTerrainPassEffect>(L"../StenGine/FX/DeferredGeometryTerrainPass");
	m_terrainShadowMapEffect = std::make_unique<TerrainShadowMapEffect>(L"../StenGine/FX/DeferredGeometryTerrainPass");
	m_vblurEffect = std::make_unique<VBlurEffect>(L"../StenGine/FX/VBlur");
	m_hblurEffect = std::make_unique<HBlurEffect>(L"../StenGine/FX/HBlur");
	m_blurEffect = std::make_unique<BlurEffect>(L"../StenGine/FX/Blur");
	m_imguiEffect = std::make_unique<ImGuiEffect>(L"../StenGine/FX/imgui");
}

EffectsManager::~EffectsManager()
{

}

}