#include "stdafx.h"

#include "imgui.h"
#include "Graphics/UI/ImGuiMenu.h"
#include "Graphics/Abstraction/ContextState.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Effect/EffectsManager.h"


namespace StenGine
{

class GLImGuiMenuImpl : public ImGuiMenu
{
public:
	GLImGuiMenuImpl()
	{
		CreateDeviceObjects();

		// TODO
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = 1280;
		io.DisplaySize.y = 720;

		io.RenderDrawListsFn = _RenderDrawLists;
	}

	~GLImGuiMenuImpl()
	{
		InvalidateDeviceObjects();
	}

	virtual void RenderDrawLists(ImDrawData* draw_data) override
	{
		ImGuiIO& io = ImGui::GetIO();
		int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
		int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
		if (fb_width == 0 || fb_height == 0)
			return;
		draw_data->ScaleClipRects(io.DisplayFramebufferScale);


		DrawCmd stateCmd;

		stateCmd.flags = CmdFlag::SET_BS | CmdFlag::SET_DS | CmdFlag::SET_CS | CmdFlag::SET_VP | CmdFlag::BIND_FB | CmdFlag::CLEAR_DEPTH;

		// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
		BlendState &blendState = stateCmd.blendState;
		blendState.index = 0;
		blendState.blendEnable = true;
		blendState.srcBlend = BlendState::Blend::BLEND_SRC_ALPHA;
		blendState.destBlend = BlendState::Blend::BLEND_INV_SRC_ALPHA;
		blendState.blendOpColor = BlendState::BlendOp::BLEND_OP_ADD;

		DepthState &depthState = stateCmd.depthState;
		depthState.depthCompEnable = false;
		depthState.depthWriteEnable = false;
		
		RasterizerState &cullState = stateCmd.rasterizerState;
		cullState.cullFaceEnabled = false;

		// Setup viewport, orthographic projection matrix
		Viewport &viewport = stateCmd.viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(fb_width);
		viewport.Height = static_cast<float>(fb_height);

		stateCmd.framebuffer = m_defaultRT;

		Renderer::Instance()->AddDeferredDrawCmd(stateCmd);

		const Mat4 ortho_projection =
		{
			{ 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
			{ 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
			{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
			{ -1.0f,                  1.0f,                   0.0f, 1.0f },
		};

		ImGuiEffect* effect = EffectsManager::Instance()->m_imguiEffect.get();

		DrawCmd updateVBIBCmd;

		std::vector<ImDrawIdx>     &IdxBuffer = updateVBIBCmd.imGuiIdxBuffer;
		std::vector<ImDrawVert>    &VtxBuffer = updateVBIBCmd.imGuiVtxBuffer;

		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];

			for (int32_t i = 0; i < cmd_list->VtxBuffer.size(); i++)
			{
				VtxBuffer.push_back(cmd_list->VtxBuffer[i]);
			}

			for (int32_t i = 0; i < cmd_list->IdxBuffer.size(); i++)
			{
				IdxBuffer.push_back(cmd_list->IdxBuffer[i]);
			}
		}

		updateVBIBCmd.imGuiVbo = m_vbo;
		updateVBIBCmd.imGuiIbo = m_ibo;

		Renderer::Instance()->AddDeferredDrawCmd(updateVBIBCmd);

		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;

		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			const ImDrawIdx* idx_buffer_offset = 0;

			for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++)
			{
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					DrawCmd cmd;

					cmd.flags = CmdFlag::DRAW | CmdFlag::SET_SS;

					ScissorState &scissorState = cmd.scissorState;

					scissorState.scissorTestEnabled = true;
					scissorState.x = (int32_t)pcmd->ClipRect.x;
					scissorState.y = (int32_t)(fb_height - pcmd->ClipRect.w);
					scissorState.width = (int32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
					scissorState.height = (int32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);

					ConstantBuffer cbuffer0 = Renderer::Instance()->CreateConstantBuffer(0, sizeof(ImGuiEffect::IMGUI_CONSTANT_BUFFER), effect->m_imguiCB);
					ImGuiEffect::IMGUI_CONSTANT_BUFFER* imguiData = (ImGuiEffect::IMGUI_CONSTANT_BUFFER*)cbuffer0->GetBuffer();

					ConstantBuffer cbuffer1 = Renderer::Instance()->CreateConstantBuffer(1, sizeof(ImGuiEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER), effect->m_textureCB);
					ImGuiEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER* textureData = (ImGuiEffect::BINDLESS_TEXTURE_CONSTANT_BUFFER*)cbuffer1->GetBuffer();

					textureData->Texture = (uint64_t)pcmd->TextureId;
					imguiData->ProjMtx = ortho_projection;

					cmd.cbuffers.push_back(std::move(cbuffer0));
					cmd.cbuffers.push_back(std::move(cbuffer1));
					cmd.drawType = DrawType::INDEXED;
					cmd.type = PrimitiveTopology::TRIANGLELIST;
					cmd.vertexBuffer.push_back((void*)m_vbo);
					cmd.indexBuffer = (void*)m_ibo;
					cmd.inputLayout = effect->GetInputLayout();
					cmd.effect = effect;
					cmd.elementCount = (int64_t)pcmd->ElemCount;
					cmd.offset = (void*)(idx_buffer_offset + indexOffset);
					cmd.vertexStride.push_back(sizeof(ImDrawVert));
					cmd.vertexOffset.push_back(sizeof(ImDrawVert) * vertexOffset);

					Renderer::Instance()->AddDeferredDrawCmd(cmd);
				}
				idx_buffer_offset += pcmd->ElemCount;
			}

			vertexOffset += cmd_list->VtxBuffer.size();
			indexOffset += cmd_list->IdxBuffer.size();
		}
	}

private:
	bool CreateFontTexture()
	{
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

		glCreateTextures(GL_TEXTURE_2D, 1, &m_fontTexture);
		glTextureParameteri(m_fontTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_fontTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glTextureStorage2D(m_fontTexture, 1, GL_RGBA8, width, height);
		
		GLuint pbo;
		glCreateBuffers(1, &pbo);
		glNamedBufferData(pbo, width * height * 4, nullptr, GL_STREAM_DRAW);
		void* pboData = glMapNamedBuffer(pbo, GL_WRITE_ONLY);
		
		memcpy(pboData, pixels, width * height * 4);
		
		glUnmapNamedBuffer(pbo);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		
		glTextureSubImage2D(
			m_fontTexture, 0,
			0, 0,
			width,
			height,
			GL_RGBA, GL_UNSIGNED_BYTE,
			(void*)0);
		
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glDeleteBuffers(1, &pbo);

		m_fontTextureHandle = glGetTextureHandleARB(m_fontTexture);
		glMakeTextureHandleResidentARB(m_fontTextureHandle);

		io.Fonts->TexID = (void*)m_fontTextureHandle;

		return true;
	}

	bool CreateDeviceObjects()
	{
		CreateFontTexture();

		glCreateBuffers(1, &m_vbo);
		glCreateBuffers(1, &m_ibo);

		m_defaultRT = Renderer::Instance()->CreateRenderTarget();
		m_defaultRT->Set(0);

		return true;
	}

	void InvalidateDeviceObjects()
	{
		glDeleteBuffers(1, &m_ibo);
		glDeleteBuffers(1, &m_vbo);

		glMakeTextureHandleNonResidentARB(m_fontTextureHandle);
		glDeleteTextures(1, &m_fontTexture);
	}

	static void _RenderDrawLists(ImDrawData* draw_data)
	{
		ImGuiMenu::Instance()->RenderDrawLists(draw_data);
	}

	bool HandleMsg(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		ImGuiIO& io = ImGui::GetIO();
		switch (msg)
		{
		case WM_LBUTTONDOWN:
			io.MouseDown[0] = true;
			return true;
		case WM_LBUTTONUP:
			io.MouseDown[0] = false;
			return true;
		case WM_RBUTTONDOWN:
			io.MouseDown[1] = true;
			return true;
		case WM_RBUTTONUP:
			io.MouseDown[1] = false;
			return true;
		case WM_MBUTTONDOWN:
			io.MouseDown[2] = true;
			return true;
		case WM_MBUTTONUP:
			io.MouseDown[2] = false;
			return true;
		case WM_MOUSEWHEEL:
			io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
			return true;
		case WM_MOUSEMOVE:
			io.MousePos.x = (signed short)(lParam);
			io.MousePos.y = (signed short)(lParam >> 16);
			return true;
		case WM_KEYDOWN:
			if (wParam < 256)
				io.KeysDown[wParam] = 1;
			return true;
		case WM_KEYUP:
			if (wParam < 256)
				io.KeysDown[wParam] = 0;
			return true;
		case WM_CHAR:
			// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
			if (wParam > 0 && wParam < 0x10000)
				io.AddInputCharacter((unsigned short)wParam);
			return true;
		}
		return false;
	}

	GLuint       m_fontTexture;
	uint64_t	 m_fontTextureHandle;

	GLuint		 m_vbo;
	GLuint		 m_ibo;

	RenderTarget m_defaultRT;
};

namespace detail
{
ImGuiMenu* CreateOpenGLImGuiMenu()
{
	auto imguiMenu = new GLImGuiMenuImpl();
	return static_cast<ImGuiMenu*>(imguiMenu);
}
}

}
