#pragma once

#include "Direct3D.h"
#include "./ImGui/imgui_impl_win32.h"


class Renderer
{
private:
	HWND m_hWnd;
	Direct3D* m_pd3d;
	ImGuiWindowFlags m_windowFlags;

public:
	~Renderer();
	void SetupContext(HWND hWnd);
	void UpdateRenderPos();
	void Render();
	void Shutdown();
};

