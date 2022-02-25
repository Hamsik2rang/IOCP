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

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("./assets/fonts/OpenSans/OpenSans-Regular.ttf", 20.0f);
	(void)io;

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
	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
	{
		counter++;
	}
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);
	char temp[] = "Lorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\nLorem Ipsum\n";
	ImGui::InputTextMultiline("Chat", temp, ARRAYSIZE(temp), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 20), ImGuiInputTextFlags_ReadOnly | ImGuiWindowFlags_AlwaysVerticalScrollbar);
	static char inputBuf[64]="";
	if (ImGui::BeginTable("Input Table", 2))
	{
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 300.0f);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 100.0f);
		ImGui::TableNextColumn(); ImGui::InputTextMultiline("Input", (char*)inputBuf, ARRAYSIZE(inputBuf), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4.8));
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


