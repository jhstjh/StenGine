#include "GLESRenderer.h"
#include "AndroidType.h"
#include "EffectsManager.h"

GLESRenderer* GLESRenderer::_instance = nullptr;

GLESRenderer::GLESRenderer() {
	_instance = this;
}

GLESRenderer::~GLESRenderer() {

}

bool GLESRenderer::Init() {
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CW); // GL_CCW for counter clock-wise
	glDepthMask(GL_TRUE);
	//glViewport(0, 0, m_clientWidth, m_clientHeight);

	// init grid and coord debug draw
	std::vector<XMFLOAT3> coordVertexBuffer = {
		XMFLOAT3(0, 0, 0),
		XMFLOAT3(5, 0, 0),
		XMFLOAT3(0, 0, 0),
		XMFLOAT3(0, 5, 0),
		XMFLOAT3(0, 0, 0),
		XMFLOAT3(0, 0, 5),
	};

	std::vector<UINT> coordIndexBuffer = { 0, 1, 2, 3, 4, 5 };


	int initIdx = 6;
	for (int i = 0; i <= 10; i++) {
		coordVertexBuffer.push_back(XMFLOAT3(-5, 0, -5 + i));
		coordVertexBuffer.push_back(XMFLOAT3(5, 0, -5 + i));
		coordIndexBuffer.push_back(initIdx++);
		coordIndexBuffer.push_back(initIdx++);
	}

	for (int i = 0; i <= 10; i++) {
		coordVertexBuffer.push_back(XMFLOAT3(-5 + i, 0, -5));
		coordVertexBuffer.push_back(XMFLOAT3(-5 + i, 0, 5));
		coordIndexBuffer.push_back(initIdx++);
		coordIndexBuffer.push_back(initIdx++);
	}

	GLuint debugDrawVertexVBO;
	glGenBuffers(1, &debugDrawVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, debugDrawVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, coordVertexBuffer.size() * sizeof(XMFLOAT3), &coordVertexBuffer[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_debugCoordVAO);
	glBindVertexArray(m_debugCoordVAO);
	glBindBuffer(GL_ARRAY_BUFFER, debugDrawVertexVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	return true;
}

void GLESRenderer::Draw() {
	DrawDebug();
}

void GLESRenderer::DrawDebug() {
	glDepthMask(GL_FALSE);
	// copy depth into default depth buffer
// 	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
// 	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_deferredGBuffers);
// 	glBlitFramebuffer(0, 0, m_clientWidth, m_clientHeight,
// 		0, 0, m_clientWidth, m_clientHeight,
// 		GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	DebugLineEffect* debugLineFX = EffectsManager::Instance()->m_debugLineEffect;
	debugLineFX->SetShader();
	glBindVertexArray(m_debugCoordVAO);
	debugLineFX->m_perObjUniformBuffer.ViewProj = ndk_helper::Mat4::Identity();//CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix();
	debugLineFX->UpdateConstantBuffer();
	glDrawArrays(GL_LINES, 6, 44);
	glDrawArrays(GL_LINES, 0, 6);
	//debugLineFX->UnSetShader();
	//debugLineFX->UnBindShaderResource();
	glDepthMask(GL_TRUE);
}