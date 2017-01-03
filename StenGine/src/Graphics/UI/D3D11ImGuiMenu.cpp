#include "imgui.h"
#include "Graphics/UI/ImGuiMenu.h"
#include "Graphics/Abstraction/ContextState.h"
#include "Graphics/Abstraction/RendererBase.h"
#include "Graphics/Effect/EffectsManager.h"
#include "Graphics/D3DIncludes.h"
#include <stdint.h>

namespace StenGine
{

class D3D11ImGuiMenuImpl : public ImGuiMenu
{
public:
	D3D11ImGuiMenuImpl()
	{
		CreateDeviceObjects();

		// TODO
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = 1280;
		io.DisplaySize.y = 720;

		io.RenderDrawListsFn = _RenderDrawLists;
	}

	~D3D11ImGuiMenuImpl()
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

		stateCmd.flags = CmdFlag::SET_BS | CmdFlag::SET_DS | CmdFlag::SET_CS | CmdFlag::SET_VP /*| CmdFlag::BIND_FB | CmdFlag::CLEAR_DEPTH*/;

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
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		//stateCmd.framebuffer = 0;

		Renderer::Instance()->AddDeferredDrawCmd(stateCmd);

		const float L = 0.0f;
		const float R = ImGui::GetIO().DisplaySize.x;
		const float B = ImGui::GetIO().DisplaySize.y;
		const float T = 0.0f;
		const DirectX::XMMATRIX ortho_projection =
		{
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
		};

		ImGuiEffect* effect = EffectsManager::Instance()->m_imguiEffect.get();

		// Create and grow vertex/index buffers if needed
		if (!m_vb || m_vbSize < draw_data->TotalVtxCount)
		{
			if (m_vb) { m_vb->Release(); m_vb = NULL; }
			m_vbSize = draw_data->TotalVtxCount + 5000;
			D3D11_BUFFER_DESC desc;
			memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.ByteWidth = m_vbSize * sizeof(ImDrawVert);
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			if (static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&desc, NULL, &m_vb) < 0)
				return;
		}
		if (!m_ib || m_ibSize < draw_data->TotalIdxCount)
		{
			if (m_ib) { m_ib->Release(); m_ib = NULL; }
			m_ibSize = draw_data->TotalIdxCount + 10000;
			D3D11_BUFFER_DESC bufferDesc;
			memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth = m_ibSize * sizeof(ImDrawIdx);
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			if (static_cast<ID3D11Device*>(Renderer::Instance()->GetDevice())->CreateBuffer(&bufferDesc, NULL, &m_ib) < 0)
				return;
		}

		// Copy and convert all vertices into a single contiguous buffer
		D3D11_MAPPED_SUBRESOURCE vtx_resource, idx_resource;
		if (static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource) != S_OK)
			return;
		if (static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Map(m_ib, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource) != S_OK)
			return;
		ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource.pData;
		ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource.pData;
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			memcpy(vtx_dst, &cmd_list->VtxBuffer[0], cmd_list->VtxBuffer.size() * sizeof(ImDrawVert));
			memcpy(idx_dst, &cmd_list->IdxBuffer[0], cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx));
			vtx_dst += cmd_list->VtxBuffer.size();
			idx_dst += cmd_list->IdxBuffer.size();
		}
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_vb, 0);
		static_cast<ID3D11DeviceContext*>(Renderer::Instance()->GetDeviceContext())->Unmap(m_ib, 0);


		uint32_t vtx_offset = 0;
		uint32_t idx_offset = 0;

		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
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
					scissorState.y = (int32_t)pcmd->ClipRect.y;
					scissorState.width = (int32_t)pcmd->ClipRect.z;
					scissorState.height = (int32_t)pcmd->ClipRect.w;

					ConstantBuffer cbuffer0(0, sizeof(ImGuiEffect::IMGUI_CONSTANT_BUFFER), effect->m_imguiCB);
					ImGuiEffect::IMGUI_CONSTANT_BUFFER* imguiData = (ImGuiEffect::IMGUI_CONSTANT_BUFFER*)cbuffer0.GetBuffer();

					imguiData->ProjMtx = ortho_projection;

					cmd.srvs.AddSRV((ID3D11ShaderResourceView*)pcmd->TextureId, 0);
					cmd.cbuffers.push_back(std::move(cbuffer0));
					cmd.drawType = DrawType::INDEXED;
					cmd.type = PrimitiveTopology::TRIANGLELIST;
					cmd.vertexBuffer = m_vb;
					cmd.indexBuffer = m_ib;
					cmd.inputLayout = effect->GetInputLayout();
					cmd.effect = effect;
					cmd.elementCount = (int64_t)pcmd->ElemCount;
					cmd.offset = (void*)(idx_offset);
					cmd.vertexStride = sizeof(ImDrawVert);
					cmd.vertexOffset = vtx_offset;

					Renderer::Instance()->AddDeferredDrawCmd(cmd);
				}
				idx_offset += pcmd->ElemCount;
			}

			vtx_offset += cmd_list->VtxBuffer.size();
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
		return true;
	}

	void InvalidateDeviceObjects()
	{
		SafeRelease(m_fontSampler);
		SafeRelease(m_fontTextureSRV);
		SafeRelease(m_vb);
		SafeRelease(m_ib);
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

	ID3D11Buffer*		 m_vb = nullptr;
	ID3D11Buffer*		 m_ib = nullptr;

	uint32_t			 m_vbSize = 0;
	uint32_t			 m_ibSize = 0;
};

namespace detail
{
ImGuiMenu* CreateD3D11ImGuiMenu()
{
	auto imguiMenu = new D3D11ImGuiMenuImpl();
	return static_cast<ImGuiMenu*>(imguiMenu);
}
}

}
