#include "EffectsManager.h"
#include "D3D11Renderer.h"
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
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateVertexShader(
			m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(),
			nullptr,
			&m_vertexShader
		);
		assert(SUCCEEDED(hr));
	}

	if (psPath.length()) {
		ReadShaderFile(psPath, &m_psBlob, "ps_5_0");
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreatePixelShader(
			m_psBlob->GetBufferPointer(),
			m_psBlob->GetBufferSize(),
			nullptr,
			&m_pixelShader
		);
		assert(SUCCEEDED(hr));
	}

	if (gsPath.length()) {
		ReadShaderFile(gsPath, &m_gsBlob, "gs_5_0");
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateGeometryShader(
			m_gsBlob->GetBufferPointer(),
			m_gsBlob->GetBufferSize(),
			nullptr,
			&m_geometryShader
			);
		assert(SUCCEEDED(hr));
	}

	if (hsPath.length()) {
		ReadShaderFile(hsPath, &m_hsBlob, "hs_5_0");
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateHullShader(
			m_hsBlob->GetBufferPointer(),
			m_hsBlob->GetBufferSize(),
			nullptr,
			&m_hullShader
			);
		assert(SUCCEEDED(hr));
	}

	if (dsPath.length()) {
		ReadShaderFile(dsPath, &m_dsBlob, "ds_5_0");
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateDomainShader(
			m_dsBlob->GetBufferPointer(),
			m_dsBlob->GetBufferSize(),
			nullptr,
			&m_domainShader
			);
		assert(SUCCEEDED(hr));
	}

	if (csPath.length()) {
		ReadShaderFile(csPath, &m_csBlob, "cs_5_0");
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateComputeShader(
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
			assert(false);
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
			assert(false);
			// 			fprintf(stderr, "ERROR: GL shader index %i did not compile\n", m_vertexShader);
			// 			_print_shader_info_log(vs);
		}
	}

	m_shaderProgram = glCreateProgram();
	glAttachShader(m_shaderProgram, m_pixelShader);
	glAttachShader(m_shaderProgram, m_vertexShader);
	glLinkProgram(m_shaderProgram);

	/* check for shader linking errors - very important! */
	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
		assert(false);
// 		fprintf(
// 			stderr,
// 			"ERROR: could not link shader programme GL index %i\n",
// 			shader_programme
// 			);
// 		_print_programme_info_log(shader_programme);
// 		return 1;
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
	D3D11Renderer::Instance()->GetD3DContext()->VSSetConstantBuffers(0, 0, nullptr);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetConstantBuffers(0, 0, nullptr);
	D3D11Renderer::Instance()->GetD3DContext()->HSSetConstantBuffers(0, 0, nullptr);
	D3D11Renderer::Instance()->GetD3DContext()->DSSetConstantBuffers(0, 0, nullptr);
	D3D11Renderer::Instance()->GetD3DContext()->GSSetConstantBuffers(0, 0, nullptr);
	D3D11Renderer::Instance()->GetD3DContext()->CSSetConstantBuffers(0, 0, nullptr);
#else
// gl
#endif
}

void Effect::UnBindShaderResource() {
#ifdef GRAPHICS_D3D11
	static ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	D3D11Renderer::Instance()->GetD3DContext()->VSSetShaderResources(0, 16, nullSRV);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShaderResources(0, 16, nullSRV);
	D3D11Renderer::Instance()->GetD3DContext()->HSSetShaderResources(0, 16, nullSRV);
	D3D11Renderer::Instance()->GetD3DContext()->DSSetShaderResources(0, 16, nullSRV);
	D3D11Renderer::Instance()->GetD3DContext()->GSSetShaderResources(0, 16, nullSRV);
	D3D11Renderer::Instance()->GetD3DContext()->CSSetShaderResources(0, 16, nullSRV);
#else
// gl
#endif
}



void Effect::SetShader() {
#ifdef GRAPHICS_D3D11
	//if (m_vertexShader)
		D3D11Renderer::Instance()->GetD3DContext()->VSSetShader(m_vertexShader, 0, 0);
	//if (m_pixelShader)
		D3D11Renderer::Instance()->GetD3DContext()->PSSetShader(m_pixelShader, 0, 0);
	//if (m_geometryShader)
		D3D11Renderer::Instance()->GetD3DContext()->GSSetShader(m_geometryShader, 0, 0);
	//if (m_hullShader)
		D3D11Renderer::Instance()->GetD3DContext()->HSSetShader(m_hullShader, 0, 0);
	//if (m_domainShader)
		D3D11Renderer::Instance()->GetD3DContext()->DSSetShader(m_domainShader, 0, 0);
		//if (m_domainShader)
		D3D11Renderer::Instance()->GetD3DContext()->CSSetShader(m_computeShader, 0, 0);
#else
	// gl
	glUseProgram(m_shaderProgram);
#endif
}

void Effect::UnSetShader() {
#ifdef GRAPHICS_D3D11
	D3D11Renderer::Instance()->GetD3DContext()->VSSetShader(nullptr, 0, 0);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShader(nullptr, 0, 0);
	D3D11Renderer::Instance()->GetD3DContext()->HSSetShader(nullptr, 0, 0);
	D3D11Renderer::Instance()->GetD3DContext()->GSSetShader(nullptr, 0, 0);
	D3D11Renderer::Instance()->GetD3DContext()->DSSetShader(nullptr, 0, 0);
	D3D11Renderer::Instance()->GetD3DContext()->CSSetShader(nullptr, 0, 0);
#else
	// gl
#endif
}

#ifdef GRAPHICS_D3D11
void Effect::UnbindUnorderedAccessViews() {
	static ID3D11UnorderedAccessView* nullUAV[7] = { 0 };
	D3D11Renderer::Instance()->GetD3DContext()->CSSetUnorderedAccessViews(0, 7, nullUAV, 0);
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

//------------------------------------------------------------//


// StdMeshEffect::StdMeshEffect(const std::wstring& filename)
// 	: Effect(filename)
// {
// 	StdMeshTech = m_fx->GetTechniqueByName("StdMeshTech");
// 	WorldViewProj = m_fx->GetVariableByName("gWorldViewProj")->AsMatrix();
// 	WorldInvTranspose = m_fx->GetVariableByName("gWorldInvTranspose")->AsMatrix();
// 	ShadowTransform = m_fx->GetVariableByName("gShadowTransform")->AsMatrix();
// 	World = m_fx->GetVariableByName("gWorld")->AsMatrix();
// 	DirLight = m_fx->GetVariableByName("gDirLight");
// 	Mat = m_fx->GetVariableByName("gMaterial");
// 	EyePosW = m_fx->GetVariableByName("gEyePosW")->AsVector();
// 	DiffuseMap = m_fx->GetVariableByName("gDiffuseMap")->AsShaderResource();
// 	TheShadowMap = m_fx->GetVariableByName("gShadowMap")->AsShaderResource();
// 
// 	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
// 	{
// 		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
// 		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
// 		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
// 		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
// 	};
// 
// 	D3DX11_PASS_DESC passDesc;
// 	StdMeshTech->GetPassByIndex(0)->GetDesc(&passDesc);
// 	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 4, passDesc.pIAInputSignature,
// 	passDesc.IAInputSignatureSize, &m_inputLayout));
// 
// 	m_activeTech = StdMeshTech;
// }
// 
// StdMeshEffect::~StdMeshEffect()
// {
// 	ReleaseCOM(m_inputLayout);
// }


//----------------------------------------------------------//


ShadowMapEffect::ShadowMapEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + EXT, L"")
{

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
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
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
			&m_perObjectCB);

		assert(SUCCEEDED(hr));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
}

void ShadowMapEffect::UpdateConstantBuffer() {
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		D3D11Renderer::Instance()->GetD3DContext()->Map(m_perObjectCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perObjConstantBuffer, sizeof(PEROBJ_CONSTANT_BUFFER));
		D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_perObjectCB, NULL);
	}
}

void ShadowMapEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perObjectCB };
	D3D11Renderer::Instance()->GetD3DContext()->VSSetConstantBuffers(0, 1, cbuf);
}

ShadowMapEffect::~ShadowMapEffect()
{
	ReleaseCOM(m_inputLayout);
}


//------------------------------------------------------------//


DeferredGeometryPassEffect::DeferredGeometryPassEffect(const std::wstring& vsPath, const std::wstring& psPath, const std::wstring& gsPath, const std::wstring& hsPath, const std::wstring& dsPath) 
	:Effect(vsPath, psPath, gsPath, hsPath, dsPath)
{

}

DeferredGeometryPassEffect::DeferredGeometryPassEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + EXT, filename + L"_ps" + EXT)
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 4, m_vsBlob->GetBufferPointer(),
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
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
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
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
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

DeferredGeometryPassEffect::~DeferredGeometryPassEffect()
{
	ReleaseCOM(m_inputLayout);
}

void DeferredGeometryPassEffect::UpdateConstantBuffer() {
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		D3D11Renderer::Instance()->GetD3DContext()->Map(m_perObjectCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perObjConstantBuffer, sizeof(PEROBJ_CONSTANT_BUFFER));
		D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_perObjectCB, NULL);
	}

	{
		D3D11_MAPPED_SUBRESOURCE ms;
		D3D11Renderer::Instance()->GetD3DContext()->Map(m_perFrameCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perFrameConstantBuffer, sizeof(PERFRAME_CONSTANT_BUFFER));
		D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_perFrameCB, NULL);
	}
}

void DeferredGeometryPassEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perObjectCB, m_perFrameCB };
	D3D11Renderer::Instance()->GetD3DContext()->VSSetConstantBuffers(0, 2, cbuf);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetConstantBuffers(0, 2, cbuf);
}

void DeferredGeometryPassEffect::BindShaderResource() {
	D3D11Renderer::Instance()->GetD3DContext()->VSSetShaderResources(0, 5, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShaderResources(0, 5, m_shaderResources);
}


//------------------------------------------------------------//


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

	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 4, m_vsBlob->GetBufferPointer(),
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
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
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
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
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
		D3D11Renderer::Instance()->GetD3DContext()->Map(m_perObjectCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perObjConstantBuffer, sizeof(PEROBJ_CONSTANT_BUFFER));
		D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_perObjectCB, NULL);
	}

	{	
		D3D11_MAPPED_SUBRESOURCE ms;
		D3D11Renderer::Instance()->GetD3DContext()->Map(m_perFrameCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perFrameConstantBuffer, sizeof(PERFRAME_CONSTANT_BUFFER));
		D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_perFrameCB, NULL);
	}
}

void DeferredGeometryTessPassEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perObjectCB, m_perFrameCB };
	D3D11Renderer::Instance()->GetD3DContext()->VSSetConstantBuffers(0, 2, cbuf);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetConstantBuffers(0, 2, cbuf);
	D3D11Renderer::Instance()->GetD3DContext()->HSSetConstantBuffers(0, 2, cbuf);
	D3D11Renderer::Instance()->GetD3DContext()->DSSetConstantBuffers(0, 2, cbuf);
}

void DeferredGeometryTessPassEffect::BindShaderResource() {
	D3D11Renderer::Instance()->GetD3DContext()->VSSetShaderResources(0, 5, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShaderResources(0, 5, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->HSSetShaderResources(0, 5, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->DSSetShaderResources(0, 5, m_shaderResources);
}


//----------------------------------------------------------//


DeferredShadingPassEffect::DeferredShadingPassEffect(const std::wstring& filename)
	: Effect(std::wstring(L"FX/ScreenQuad_vs") + EXT, filename + L"_ps" + EXT)
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
		m_vsBlob->GetBufferSize(), &m_inputLayout));

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
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
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

DeferredShadingPassEffect::~DeferredShadingPassEffect()
{
	ReleaseCOM(m_inputLayout);
}

void DeferredShadingPassEffect::UpdateConstantBuffer() {
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		D3D11Renderer::Instance()->GetD3DContext()->Map(m_perFrameCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perFrameConstantBuffer, sizeof(PERFRAME_CONSTANT_BUFFER));
		D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_perFrameCB, NULL);
	}
}

void DeferredShadingPassEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perFrameCB };
	//D3D11Renderer::Instance()->GetD3DContext()->VSSetConstantBuffers(0, 1, cbuf);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetConstantBuffers(0, 1, cbuf);
}

void DeferredShadingPassEffect::BindShaderResource() {
	D3D11Renderer::Instance()->GetD3DContext()->VSSetShaderResources(0, 4, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShaderResources(0, 4, m_shaderResources);
}


//----------------------------------------------------------//

// ScreenQuadEffect::ScreenQuadEffect(const std::wstring& filename)
// 	: Effect(filename)
// {
// 	FullScreenQuadTech = m_fx->GetTechniqueByName("t0");
// //	SSAOTech = m_fx->GetTechniqueByName("SSAOTech");
// 	HBlurTech = m_fx->GetTechniqueByName("HBlurTech");
// 	VBlurTech = m_fx->GetTechniqueByName("VBlurTech");
// 	ScreenMap = m_fx->GetVariableByName("gScreenMap")->AsShaderResource();
// 	SSAOMap = m_fx->GetVariableByName("gSSAOMap")->AsShaderResource();
// 	DiffuseGB = m_fx->GetVariableByName("gDiffuseGB")->AsShaderResource();
// 	PositionGB = m_fx->GetVariableByName("gPositionGB")->AsShaderResource();
// 	SpecularGB = m_fx->GetVariableByName("gSpecularGB")->AsShaderResource();
// 	NormalGB = m_fx->GetVariableByName("gNormalGB")->AsShaderResource();
// 	DepthGB = m_fx->GetVariableByName("gDepthGB")->AsShaderResource();
// 	ProjInv = m_fx->GetVariableByName("gProjInv")->AsMatrix();
// 	Proj = m_fx->GetVariableByName("gProj")->AsMatrix();
// 	DirLight = m_fx->GetVariableByName("gDirLight");
// 	//Mat = m_fx->GetVariableByName("gMaterial");
// 	EyePosW = m_fx->GetVariableByName("gEyePosW")->AsVector();
// 
// 	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
// 	{
// 		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
// 		//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
// 	};
// 
// 	D3DX11_PASS_DESC passDesc;
// 	FullScreenQuadTech->GetPassByIndex(0)->GetDesc(&passDesc);
// 	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 1, passDesc.pIAInputSignature,
// 		passDesc.IAInputSignatureSize, &m_inputLayout));
// 
// 	m_activeTech = FullScreenQuadTech;
// }
// 
// ScreenQuadEffect::~ScreenQuadEffect()
// {
// 	//ReleaseCOM(m_inputLayout);
// }


//----------------------------------------------------------//


// GodRayEffect::GodRayEffect(const std::wstring& filename)
// 	: Effect(filename)
// {
// 	GodRayTech = m_fx->GetTechniqueByName("t0");
// 	OcclusionMap = m_fx->GetVariableByName("gOcclusionMap")->AsShaderResource();
// 	LightPosH = m_fx->GetVariableByName("gLightPosH")->AsVector();
// 
// 	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
// 	{
// 		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
// 		//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
// 	};
// 
// 	D3DX11_PASS_DESC passDesc;
// 	GodRayTech->GetPassByIndex(0)->GetDesc(&passDesc);
// 	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 1, passDesc.pIAInputSignature,
// 		passDesc.IAInputSignatureSize, &m_inputLayout));
// 
// 	m_activeTech = GodRayTech;
// }
// 
// GodRayEffect::~GodRayEffect()
// {
// 	//ReleaseCOM(m_inputLayout);
// }

//----------------------------------------------------------//


SkyboxEffect::SkyboxEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + EXT, filename + L"_ps" + EXT)
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	m_shaderResources = new ID3D11ShaderResourceView*[1];
	for (int i = 0; i < 1; i++) {
		m_shaderResources[i] = 0;
	}

	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
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
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
			&m_perObjectCB);

		assert(SUCCEEDED(hr));
	}

	ReleaseCOM(m_vsBlob);
	ReleaseCOM(m_psBlob);
	ReleaseCOM(m_gsBlob);
	ReleaseCOM(m_hsBlob);
	ReleaseCOM(m_dsBlob);
	ReleaseCOM(m_csBlob);
}

void SkyboxEffect::UpdateConstantBuffer() {
	{
		D3D11_MAPPED_SUBRESOURCE ms;
		D3D11Renderer::Instance()->GetD3DContext()->Map(m_perObjectCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_perObjConstantBuffer, sizeof(PEROBJ_CONSTANT_BUFFER));
		D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_perObjectCB, NULL);
	}
}

void SkyboxEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perObjectCB };
	D3D11Renderer::Instance()->GetD3DContext()->VSSetConstantBuffers(0, 1, cbuf);
}

SkyboxEffect::~SkyboxEffect()
{
	ReleaseCOM(m_inputLayout);
}

void SkyboxEffect::BindShaderResource() {
	//D3D11Renderer::Instance()->GetD3DContext()->VSSetShaderResources(0, 4, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShaderResources(0, 1, m_shaderResources);
}

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
		blurredTexDesc.Width = D3D11Renderer::Instance()->GetScreenWidth();
		blurredTexDesc.Height = D3D11Renderer::Instance()->GetScreenHeight();
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
		HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateTexture2D(&blurredTexDesc, 0, &blurredTex));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = blurredTexDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateShaderResourceView(blurredTex, &srvDesc, &m_outputShaderResources[i]));

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = blurredTexDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateUnorderedAccessView(blurredTex, &uavDesc, &m_unorderedAccessViews[i]));


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
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
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
		D3D11Renderer::Instance()->GetD3DContext()->Map(m_settingCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_settingConstantBuffer, sizeof(SETTING_CONSTANT_BUFFER));
		D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_settingCB, NULL);
	}
}

void VBlurEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_settingCB };
	D3D11Renderer::Instance()->GetD3DContext()->CSSetConstantBuffers(0, 1, cbuf);
}

void VBlurEffect::BindShaderResource(int idx) {
	D3D11Renderer::Instance()->GetD3DContext()->CSSetShaderResources(0, 1, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->CSSetUnorderedAccessViews(0, 1, &m_unorderedAccessViews[idx], 0);
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
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
			&m_perFrameCB);

		assert(SUCCEEDED(hr));
	}


	m_outputShaderResources = new ID3D11ShaderResourceView*[2];
	m_unorderedAccessViews = new ID3D11UnorderedAccessView*[2];

	for (int i = 0; i < 2; i++) {

		D3D11_TEXTURE2D_DESC shadedTexDesc;
		shadedTexDesc.Width = D3D11Renderer::Instance()->GetScreenWidth();
		shadedTexDesc.Height = D3D11Renderer::Instance()->GetScreenHeight();
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
		HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateTexture2D(&shadedTexDesc, 0, &shadedTex));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = shadedTexDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateShaderResourceView(shadedTex, &srvDesc, &m_outputShaderResources[i]));

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = shadedTexDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateUnorderedAccessView(shadedTex, &uavDesc, &m_unorderedAccessViews[i]));


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
	D3D11Renderer::Instance()->GetD3DContext()->Map(m_perFrameCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &m_perFrameConstantBuffer, sizeof(PERFRAME_CONSTANT_BUFFER));
	D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_perFrameCB, NULL);

}

void DeferredShadingCS::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_perFrameCB };
	D3D11Renderer::Instance()->GetD3DContext()->CSSetConstantBuffers(0, 1, cbuf);
}

void DeferredShadingCS::BindShaderResource(int idx) {
	D3D11Renderer::Instance()->GetD3DContext()->CSSetShaderResources(0, 4, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->CSSetUnorderedAccessViews(0, 2, &m_unorderedAccessViews[idx], 0);
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
		blurredTexDesc.Width = D3D11Renderer::Instance()->GetScreenWidth();
		blurredTexDesc.Height = D3D11Renderer::Instance()->GetScreenHeight();
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
		HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateTexture2D(&blurredTexDesc, 0, &blurredTex));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = blurredTexDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		
		HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateShaderResourceView(blurredTex, &srvDesc, &m_outputShaderResources[i]));

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = blurredTexDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		
		HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateUnorderedAccessView(blurredTex, &uavDesc, &m_unorderedAccessViews[i]));


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
			hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
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
			D3D11Renderer::Instance()->GetD3DContext()->Map(m_settingCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
			memcpy(ms.pData, &m_settingConstantBuffer, sizeof(SETTING_CONSTANT_BUFFER));
			D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_settingCB, NULL);
		}
}

void HBlurEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_settingCB };
	D3D11Renderer::Instance()->GetD3DContext()->CSSetConstantBuffers(0, 1, cbuf);
}

void HBlurEffect::BindShaderResource(int idx) {
	D3D11Renderer::Instance()->GetD3DContext()->CSSetShaderResources(0, 1, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->CSSetUnorderedAccessViews(0, 1, &m_unorderedAccessViews[idx], 0);
}


//-------------------------------------------//


BlurEffect::BlurEffect(const std::wstring& filename)
	: Effect(std::wstring(L"FX/ScreenQuad_vs") + EXT, filename + L"_ps" + EXT)
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HR(D3D11Renderer::Instance()->GetD3DDevice()->CreateInputLayout(vertexDesc, 1, m_vsBlob->GetBufferPointer(),
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
		hr = D3D11Renderer::Instance()->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,
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
			D3D11Renderer::Instance()->GetD3DContext()->Map(m_settingCB, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
			memcpy(ms.pData, &m_settingConstantBuffer, sizeof(SETTING_CONSTANT_BUFFER));
			D3D11Renderer::Instance()->GetD3DContext()->Unmap(m_settingCB, NULL);
		}
}

void BlurEffect::BindConstantBuffer() {
	ID3D11Buffer* cbuf[] = { m_settingCB };
	//D3D11Renderer::Instance()->GetD3DContext()->VSSetConstantBuffers(0, 1, cbuf);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetConstantBuffers(0, 1, cbuf);
}

void BlurEffect::BindShaderResource() {
	//	D3D11Renderer::Instance()->GetD3DContext()->VSSetShaderResources(0, 2, m_shaderResources);
	D3D11Renderer::Instance()->GetD3DContext()->PSSetShaderResources(0, 3, m_shaderResources);
}

#else
DeferredGeometryPassEffect::DeferredGeometryPassEffect(const std::wstring& vsPath, const std::wstring& psPath, const std::wstring& gsPath, const std::wstring& hsPath, const std::wstring& dsPath)
	:Effect(vsPath, psPath, gsPath, hsPath, dsPath)
{

}

DeferredGeometryPassEffect::DeferredGeometryPassEffect(const std::wstring& filename)
	: Effect(filename + L"_vs" + EXT, filename + L"_ps" + EXT)
{

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

	assert(DiffuseMapPosition > -1);
	assert(NormalMapPosition > -1);
}

DeferredGeometryPassEffect::~DeferredGeometryPassEffect()
{

}

void DeferredGeometryPassEffect::UpdateConstantBuffer() {
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

void DeferredGeometryPassEffect::BindConstantBuffer() {

}

void DeferredGeometryPassEffect::BindShaderResource() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DiffuseMap);
	glUniform1i(DiffuseMapPosition, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NormalMap);
	glUniform1i(NormalMapPosition, 1);
}


/********************************************************************/


DeferredShadingPassEffect::DeferredShadingPassEffect(const std::wstring& filename)
	: Effect(std::wstring(L"FX/ScreenQuad_vs") + EXT, filename + L"_ps" + EXT)
{
	DiffuseGMapPosition = glGetUniformLocation(m_shaderProgram, "gDiffuseGMap");
}

DeferredShadingPassEffect::~DeferredShadingPassEffect()
{

}

void DeferredShadingPassEffect::UpdateConstantBuffer() {
// TODO
}

void DeferredShadingPassEffect::BindConstantBuffer() {
// TODO
}

void DeferredShadingPassEffect::BindShaderResource() {
 	glActiveTexture(GL_TEXTURE0);
 	glBindTexture(GL_TEXTURE_2D, DiffuseGMap);
 	glUniform1i(DiffuseGMapPosition, 0);
}


/*********************************************************************/


bool Effect::ReadShaderFile(std::wstring filename, char* shaderContent, int maxLength) {
	std::string s(filename.begin(), filename.end());
	const char* lFilename = s.c_str();
	FILE* file = fopen(lFilename, "r");
	int current_len = 0;
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


//----------------------------------------------------------//
EffectsManager* EffectsManager::_instance = nullptr;
EffectsManager::EffectsManager() {
	//m_stdMeshEffect = new StdMeshEffect(L"FX/StdMesh.fxo");
#ifdef GRAPHICS_D3D11
	m_shadowMapEffect = new ShadowMapEffect(L"FX/ShadowMap");
	m_deferredGeometryPassEffect = new DeferredGeometryPassEffect(L"FX/DeferredGeometryPass");
	m_deferredGeometryTessPassEffect = new DeferredGeometryTessPassEffect(L"FX/DeferredGeometryTessPass");
	m_deferredShadingPassEffect = new DeferredShadingPassEffect(L"FX/DeferredShadingPass");
	m_deferredShadingCSEffect = new DeferredShadingCS(L"FX/DeferredShading");

	m_blurEffect = new BlurEffect(L"FX/Blur");
	m_vblurEffect = new VBlurEffect(L"FX/VBlur");
	m_hblurEffect = new HBlurEffect(L"FX/HBlur");
	//m_screenQuadEffect = new ScreenQuadEffect(L"FX/ScreenQuad.fxo");
	//m_godrayEffect = new GodRayEffect(L"FX/GodRay.fxo");
	m_skyboxEffect = new SkyboxEffect(L"FX/Skybox");

#else
	m_deferredGeometryPassEffect = new DeferredGeometryPassEffect(L"FX/DeferredGeometryPass");
	m_deferredShadingPassEffect = new DeferredShadingPassEffect(L"FX/DeferredShadingPass");
#endif
}

EffectsManager::~EffectsManager() {
	//for (int i = 0; i < m_effects.size(); i++) {
	//	delete m_effects[i];
	//}
	//SafeDelete(m_stdMeshEffect);
	SafeDelete(m_shadowMapEffect);
	SafeDelete(m_deferredGeometryPassEffect);
	SafeDelete(m_deferredShadingPassEffect);
	//SafeDelete(m_screenQuadEffect);
	//SafeDelete(m_godrayEffect);
	SafeDelete(m_skyboxEffect);
	SafeDelete(m_blurEffect);
	SafeDelete(m_vblurEffect);
}