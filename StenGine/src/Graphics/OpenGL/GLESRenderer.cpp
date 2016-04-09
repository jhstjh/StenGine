#include "GLESRenderer.h"
#include "AndroidType.h"
#include "EffectsManager.h"
#include "CameraManager.h"

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
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
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

	std::vector<XMFLOAT4> coordVertexColorBuffer = {
		XMFLOAT4(1, 0, 0, 1),
		XMFLOAT4(1, 0, 0, 1),
		XMFLOAT4(0, 1, 0, 1),
		XMFLOAT4(0, 1, 0, 1),
		XMFLOAT4(0, 0, 1, 1),
		XMFLOAT4(0, 0, 1, 1),
	};

	std::vector<UINT> coordIndexBuffer = { 0, 1, 2, 3, 4, 5 };


	int initIdx = 6;
	for (int i = 0; i <= 10; i++) {
		coordVertexBuffer.push_back(XMFLOAT3(-5, 0, -5 + i));
		coordVertexBuffer.push_back(XMFLOAT3(5, 0, -5 + i));
		coordVertexColorBuffer.push_back(XMFLOAT4(0.5, 0.5, 0.5, 1));
		coordVertexColorBuffer.push_back(XMFLOAT4(0.5, 0.5, 0.5, 1));
		coordIndexBuffer.push_back(initIdx++);
		coordIndexBuffer.push_back(initIdx++);
	}

	for (int i = 0; i <= 10; i++) {
		coordVertexBuffer.push_back(XMFLOAT3(-5 + i, 0, -5));
		coordVertexBuffer.push_back(XMFLOAT3(-5 + i, 0, 5));
		coordVertexColorBuffer.push_back(XMFLOAT4(0.5, 0.5, 0.5, 1));
		coordVertexColorBuffer.push_back(XMFLOAT4(0.5, 0.5, 0.5, 1));
		coordIndexBuffer.push_back(initIdx++);
		coordIndexBuffer.push_back(initIdx++);
	}

	GLuint debugDrawVertexVBO;
	GLuint debugDrawVertexColorVBO;
	glGenBuffers(1, &debugDrawVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, debugDrawVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, coordVertexBuffer.size() * sizeof(XMFLOAT3), &coordVertexBuffer[0], GL_STATIC_DRAW);

	glGenBuffers(1, &debugDrawVertexColorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, debugDrawVertexColorVBO);
	glBufferData(GL_ARRAY_BUFFER, coordVertexColorBuffer.size() * sizeof(XMFLOAT4), &coordVertexColorBuffer[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_debugCoordVAO);
	glBindVertexArray(m_debugCoordVAO);
	glBindBuffer(GL_ARRAY_BUFFER, debugDrawVertexVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glBindBuffer(GL_ARRAY_BUFFER, debugDrawVertexColorVBO);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	DirectionalLight* dLight = new DirectionalLight();
	dLight->intensity = XMFLOAT4(1, 1, 1, 1);
	dLight->direction = (XMFLOAT3(-0.5, -2, 1).Normalize());
	dLight->castShadow = 1;

	LightManager::Instance()->m_dirLights.push_back(dLight);

	return true;
}

void GLESRenderer::Draw() {
	DrawMesh();
	DrawDebug();
}

void GLESRenderer::DrawMesh() {
	//glViewport(0, 0, m_clientWidth, m_clientHeight);

	EffectsManager::Instance()->m_simpleMeshEffect->SetShader();
	SimpleMeshEffect* effect = EffectsManager::Instance()->m_simpleMeshEffect;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: should separate perobj and perframe's updateconstantbuffer

	effect->m_perFrameUniformBuffer.EyePosW = (CameraManager::Instance()->GetActiveCamera()->GetPos());
	effect->m_perFrameUniformBuffer.DirLight = *LightManager::Instance()->m_dirLights[0];
// 	effect->ShadowMapTex = LightManager::Instance()->m_shadowMap->GetDepthTex();
// 	effect->CubeMapTex = m_SkyBox->m_cubeMapTex;

	for (int iMesh = 0; iMesh < EffectsManager::Instance()->m_simpleMeshEffect->m_associatedMeshes.size(); iMesh++) {
		EffectsManager::Instance()->m_simpleMeshEffect->m_associatedMeshes[iMesh]->Draw();
	}

	//EffectsManager::Instance()->m_simpleMeshEffect->UnSetShader();
}

void GLESRenderer::DrawDebug() {
	glDepthMask(GL_FALSE);
	// copy depth into default depth buffer
// 	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
// 	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_deferredGBuffers);
// 	glBlitFramebuffer(0, 0, m_clientWidth, m_clientHeight,
// 		0, 0, m_clientWidth, m_clientHeight,
// 		GL_DEPTH_BUFFER_BIT, GL_NEAREST);
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	DebugLineEffect* debugLineFX = EffectsManager::Instance()->m_debugLineEffect;
	debugLineFX->SetShader();
	glBindVertexArray(m_debugCoordVAO);
	debugLineFX->m_perObjUniformBuffer.ViewProj = CameraManager::Instance()->GetActiveCamera()->GetViewProjMatrix();
	debugLineFX->UpdateConstantBuffer();
	glDrawArrays(GL_LINES, 6, 44);
	glDrawArrays(GL_LINES, 0, 6);
	//debugLineFX->UnSetShader();
	//debugLineFX->UnBindShaderResource();
	glDepthMask(GL_TRUE);
}