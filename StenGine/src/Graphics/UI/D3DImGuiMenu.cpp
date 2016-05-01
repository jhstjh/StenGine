#if 0

#include "imgui.h"
#include "Graphics/UI/ImGuiMenu.h"
#include "Graphics/Abstraction/ContextState.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/D3DIncludes.h"
#include <stdint.h>

namespace StenGine
{

class ImGuiMenuImpl : public ImGuiMenu
{
public:
	ImGuiMenuImpl()
	{
		CreateDeviceObjects();

		// TODO
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = 1280;
		io.DisplaySize.y = 720;

		io.RenderDrawListsFn = _RenderDrawLists;
	}

	~ImGuiMenuImpl()
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

		stateCmd.framebuffer = 0;

		Renderer::Instance()->AddDeferredDrawCmd(stateCmd);

		const DirectX::XMMATRIX ortho_projection =
		{
			{ 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
			{ 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
			{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
			{ -1.0f,                  1.0f,                   0.0f, 1.0f },
		};

		ImGuiEffect* effect = EffectsManager::Instance()->m_imguiEffect.get();

		ImVector<ImDrawIdx>     IdxBuffer;
		ImVector<ImDrawVert>    VtxBuffer;

		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			const ImDrawIdx* idx_buffer_offset = 0;

			for (int32_t i = 0; i < cmd_list->VtxBuffer.size(); i++)
			{
				VtxBuffer.push_back(cmd_list->VtxBuffer[i]);
			}

			for (int32_t i = 0; i < cmd_list->IdxBuffer.size(); i++)
			{
				IdxBuffer.push_back(cmd_list->IdxBuffer[i]);
			}
		}

		glNamedBufferData(m_vbo, VtxBuffer.size() * sizeof(ImDrawVert), &VtxBuffer.front(), D3D11_STREAM_DRAW);
		glNamedBufferData(m_ibo, IdxBuffer.size() * sizeof(ImDrawIdx), &IdxBuffer.front(), D3D11_STREAM_DRAW);

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

					ConstantBuffer cbuffer0(0, sizeof(ImGuiEffect::IMGUI_CONSTANT_BUFFER), (void*)effect->m_imguiCB);
					ImGuiEffect::IMGUI_CONSTANT_BUFFER* imguiData = (ImGuiEffect::IMGUI_CONSTANT_BUFFER*)cbuffer0.GetBuffer();

					imguiData->Texture = (uint64_t)pcmd->TextureId;
					imguiData->ProjMtx = ortho_projection;

					cmd.cbuffers.push_back(std::move(cbuffer0));
					cmd.drawType = DrawType::INDEXED;
					cmd.type = PrimitiveTopology::TRIAND3D11ELIST;
					cmd.vertexBuffer = (void*)m_vbo;
					cmd.indexBuffer = (void*)m_ibo;
					cmd.inputLayout = effect->GetInputLayout();
					cmd.effect = effect;
					cmd.elementCount = (int64_t)pcmd->ElemCount;
					cmd.offset = (void*)(idx_buffer_offset + indexOffset);
					cmd.vertexStride = sizeof(ImDrawVert);
					cmd.vertexOffset = sizeof(ImDrawVert) * vertexOffset;

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
		// Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		// Upload texture to graphics system
		{
			D3D11_TEXTURE2D_DESC texDesc;
			ZeroMemory(&texDesc, sizeof(texDesc));
			texDesc.Width = width;
			texDesc.Height = height;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			texDesc.SampleDesc.Count = 1;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texDesc.CPUAccessFlags = 0;

			ID3D11Texture2D *pTexture = NULL;
			D3D11_SUBRESOURCE_DATA subResource;
			subResource.pSysMem = pixels;
			subResource.SysMemPitch = texDesc.Width * 4;
			subResource.SysMemSlicePitch = 0;
			static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateTexture2D(&texDesc, &subResource, &pTexture);

			// Create texture view
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateShaderResourceView(pTexture, &srvDesc, &m_fontTextureSRV);
			pTexture->Release();
		}

		// Store our identifier
		io.Fonts->TexID = (void *)m_fontTextureSRV;

		// Create texture sampler
		{
			D3D11_SAMPLER_DESC samplerDesc;
			ZeroMemory(&samplerDesc, sizeof(samplerDesc));
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MipLODBias = 0.f;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.MinLOD = 0.f;
			samplerDesc.MaxLOD = 0.f;
			static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateSamplerState(&samplerDesc, &m_fontSampler);
		}

		return true;
	}

	bool CreateDeviceObjects()
	{
		CreateFontTexture();

		glCreateBuffers(1, &m_vbo);
		glCreateBuffers(1, &m_ibo);

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

	ID3D11ShaderResourceView*       m_fontTextureSRV = nullptr;
	ID3D11SamplerState*				m_fontSampler = nullptr;

	ID3D11Buffer*		 m_vbo = nullptr;
	ID3D11Buffer*		 m_ibo = nullptr;
};

DEFINE_ABSTRACT_SIND3D11ETON_CLASS(ImGuiMenu, ImGuiMenuImpl)

}

#endif