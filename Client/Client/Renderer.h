#pragma once

#include "Direct3D.h"
#include "./ImGui/imgui_impl_win32.h"

#include <string>
#include <regex>

// Wrapping ImGui InputText function to print Korean.
struct CustomCallback
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

	static bool StringInputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		return ImGui::InputTextMultiline(label, (char*)str->c_str(), (size_t)str->size(), size, flags | ImGuiInputTextFlags_CallbackResize, CustomCallback::InputCallback, (void*)str);
	}

	static int FilterIPString(ImGuiInputTextCallbackData* data)
	{
		if ((data->EventChar < '0' || data->EventChar> '9') && data->EventChar != '.')
		{
			return 1;
		}
		return 0;
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

