#include "Renderer.h"

void Renderer::UpdateRenderPos()
{
	RECT rcView;
	GetClientRect(m_hWnd, &rcView);
	::ClientToScreen(m_hWnd, (POINT*)&rcView.left);
	::ClientToScreen(m_hWnd, (POINT*)&rcView.right);

	ImGui::SetNextWindowPos({(float)rcView.left, (float)rcView.top});
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
}

Renderer::~Renderer()
{
	if (m_pd3d)
	{
		delete m_pd3d;
		m_pd3d = nullptr;
	}
}

void Renderer::SetupContext(HWND hWnd)
{
	m_hWnd = hWnd;
	m_pd3d = new Direct3D;
	m_pd3d->Init(hWnd);

	unsigned char* pixel;
	int width;
	int height;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.Fonts->AddFontFromFileTTF("./assets/fonts/malgun/malgun.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesKorean());

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	m_windowFlags |= ImGuiWindowFlags_NoResize;
	m_windowFlags |= ImGuiWindowFlags_NoMove;
	m_windowFlags |= ImGuiWindowFlags_NoTitleBar;

	ImGui::StyleColorsLight();

	ImGuiStyle& style = ImGui::GetStyle();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_pd3d->GetDevice(), m_pd3d->GetDeviceContext());
}

void Renderer::Render()
{
	static float f = 0.0f;
	static int counter = 0;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	UpdateRenderPos();
	
	ImGui::Begin("Chatting Client", nullptr, m_windowFlags);
	/*if (ImGui::BeginTable("banner", 3))
	{
		ImGui::TableNextColumn();
		std::string username = u8"ÇÜ½Ä";
		std::string status = u8"Á¢¼Ó ²÷±è";
		static std::string userStr = u8"À¯Àú: " + username;
		ImGui::Text(userStr.c_str());

		ImGui::TableNextColumn();
		ImGui::Text(status.c_str());

		ImGui::TableNextColumn();
		ImGui::Button(u8"¼³Á¤", ImVec2(0.0f, ImGui::GetTextLineHeight() * 2));

		ImGui::EndTable();
	}*/
	

	ImGui::Text(u8"Á¦ÀÛ: Im Yongsik");               // Display some text (you can use a format strings too)
	ImGui::Text(u8"Username : ÇÜ½Ä");
	//ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

	//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
	//{
	//	counter++;
	//}
	//ImGui::SameLine();
	//ImGui::Text("counter = %d", counter);
	if (m_chatString.empty())
	{
		m_chatString = u8"[ÇÜ½Ä]¿À´Ã Àú³áÀº ÇØ¾¾´Ù!\n[²¿¹Ì]ÇÜ½Ä¾Æ ¹ã¿¡ ÃÂ¹ÙÄûÁ» ¾ÈÅ¸¸é ¾ÈµÇ°Ú´Ï?\n³Ê ¶§¹®¿¡ ÀáÀ» ¸øÀÚ°Ú¾î!\n[ÇÜ½Ä]³ª ´ã´Þ 15ÀÏ¿¡ ¹ÙÇÁ Âï¾î¾ß µÅ.\n±× ¶§±îÁö¸¸ Âü¾Æ!";
	}
	ResizeCallback::MyInputTextMultiline("Chat", &m_chatString, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 24), ImGuiInputTextFlags_ReadOnly | ImGuiWindowFlags_AlwaysVerticalScrollbar);
	if (ImGui::BeginTable("Input Table", 2))
	{
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 300.0f);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 100.0f);
		if (m_inputString.empty())
		{
			m_inputString.push_back('\0');
		}
		
		ImGui::TableNextColumn(); ResizeCallback::MyInputTextMultiline("Input", &m_inputString, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4.8));
		ImGui::TableNextColumn(); 
		if (ImGui::Button("Enter", ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4.8)))
		{
			// TODO: When Enter button push..
		}
		ImGui::EndTable();
	}
	ImGui::End();

	ImGui::Render();
	m_pd3d->Render();

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void Renderer::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	m_pd3d->Shutdown();
}
