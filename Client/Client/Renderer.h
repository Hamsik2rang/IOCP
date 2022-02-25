#pragma once

#include "Direct3D.h"
#include "./ImGui/imgui_impl_win32.h"

#include <string>

// Wrapping ImGui InputText function to print Korean.
struct ResizeCallback
{
	static int InputCallback(ImGuiInputTextCallbackData* data)
	{
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			std::string* str = (std::string*)data->UserData;
			if (str->empty())
			{
				str->push_back('\0');
			}
			str->resize(data->BufSize);
			data->Buf = (char*)str->c_str();
		}
		return 0;
	}

	static bool MyInputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		return ImGui::InputTextMultiline(label, (char*)str->c_str(), (size_t)str->size(), size, flags | ImGuiInputTextFlags_CallbackResize, ResizeCallback::InputCallback, (void*)str);
	}
};

class Renderer
{
private:
	HWND m_hWnd;
	Direct3D* m_pd3d;
	ImGuiWindowFlags m_windowFlags;
	std::string m_chatString;
	std::string m_inputString;

public:

	~Renderer();
	void SetupContext(HWND hWnd);
	void UpdateRenderPos();
	void Render();
	void Shutdown();
};

