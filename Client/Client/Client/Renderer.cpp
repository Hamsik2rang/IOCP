#include "Renderer.h"

void Renderer::UpdateRenderPos()
{
	RECT rcView;
	GetClientRect(m_hWnd, &rcView);
	::ClientToScreen(m_hWnd, (POINT*)&rcView.left);
	::ClientToScreen(m_hWnd, (POINT*)&rcView.right);

	//ImGuiViewport* viewport = ImGui::GetWindowViewport();
	//ImVec2 vpCenter = viewport->GetWorkCenter();
	//ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	//ImVec2 vpLeftTop = { vpCenter.x - (displaySize.x / 2), vpCenter.y - (displaySize.y)/2};
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
	(void)io;

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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
	
	ImGui::Begin("Chatting Client", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

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
	m_pd3d->Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


