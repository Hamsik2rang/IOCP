#pragma once

#include <d3d11.h>

#include "./ImGui/imgui.h"
#include "./ImGui/imgui_impl_dx11.h"


class Direct3D
{
private:
	HWND m_hWnd;

	ID3D11Device* m_pd3dDevice = nullptr;
	ID3D11DeviceContext* m_pd3dDeviceContext = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;

	void CleanupDevice();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	bool CreateDevice();
public:
	bool Init(HWND hWnd);
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void Render();
	void Shutdown();
};

