#include "Direct3D.h"

Direct3D::~Direct3D()
{
    CleanupDevice();
}

bool Direct3D::Init(HWND hWnd)
{
    m_hWnd = hWnd;
    if (!CreateDevice())
    {
        CleanupDevice();
        return false;
    }
}

ID3D11Device* Direct3D::GetDevice()
{
    return m_pd3dDevice;
}

ID3D11DeviceContext* Direct3D::GetDeviceContext()
{
    return m_pd3dDeviceContext;
}

void Direct3D::Render()
{
    const float rgba[] = {0.25f, 0.35f, 0.6f, 1.0f};
    m_pd3dDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);
    m_pd3dDeviceContext->ClearRenderTargetView(m_pRenderTargetView, rgba);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    m_pSwapChain->Present(1, 0);
}

void Direct3D::Shutdown()
{
    CleanupDevice();
}

void Direct3D::CleanupDevice()
{
    CleanupRenderTarget();
    if (m_pSwapChain)
    {
        m_pSwapChain->Release();
        m_pSwapChain = nullptr;
    }
    if (m_pd3dDeviceContext)
    {
        m_pd3dDeviceContext->Release();
        m_pd3dDevice = nullptr;
    }
    if (m_pd3dDevice)
    {
        m_pd3dDevice->Release();
        m_pd3dDevice = nullptr;
    }
}

bool Direct3D::CreateDevice()
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (S_OK != D3D11CreateDeviceAndSwapChain(NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        createDeviceFlags,
        featureLevelArray,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &m_pSwapChain,
        &m_pd3dDevice,
        &featureLevel,
        &m_pd3dDeviceContext))
    {
        return false;
    }

    CreateRenderTarget();

    return true;
}

void Direct3D::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (nullptr == pBackBuffer)
    {
        MessageBox(m_hWnd, L"Fail to Create Back Buffer", L"Error", MB_OK);
        return;
    }
    m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
    pBackBuffer->Release();
}

void Direct3D::CleanupRenderTarget()
{
    if (m_pRenderTargetView)
    {
        m_pRenderTargetView->Release();
        m_pRenderTargetView = nullptr;
    }
}