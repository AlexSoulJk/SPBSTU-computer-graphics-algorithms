#include "framework.h"
#include "Render.h"

#include <filesystem>

#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")

struct Vertex
{
    float x, y, z;
    COLORREF color;
};

struct MatrixBuffer
{
    XMMATRIX m;
};


Render::Render(HWND hWnd) : m_hWnd(hWnd), camera(nullptr), m_currentModel(nullptr),
    m_pDevice(nullptr),
    m_pDeviceContext(nullptr),
    m_pSwapChain(nullptr),
    m_pRenderTargetView(nullptr),
    m_pPixelShader(nullptr),
    m_pVertexShader(nullptr),
    m_pInputLayout(nullptr),
    m_szTitle(nullptr),
    m_szWindowClass(nullptr){}

HRESULT Render::Init(WCHAR szTitle[], WCHAR szWindowClass[])
{
    m_szTitle = szTitle;
    m_szWindowClass = szWindowClass;

    HRESULT hr;
    ModelFactory::ModelCode init_code = ModelFactory::ModelCode::cube;

    IDXGIFactory* pFactory = nullptr;
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);

    IDXGIAdapter* pAdapter = nullptr;
    hr = pFactory->EnumAdapters(0, &pAdapter);
    if (FAILED(hr)) {
        OutputDebugString(L"[ERROR] Не удалось получить графический адаптер\n");
        pFactory->Release();
        return hr;
    }

    // Создание устройства Direct3D 11
    D3D_FEATURE_LEVEL featureLevel;
    hr = D3D11CreateDevice(
        pAdapter,
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
#ifdef _DEBUG
        D3D11_CREATE_DEVICE_DEBUG,
#else
        0,
#endif
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &m_pDevice,
        &featureLevel,
        &m_pDeviceContext
    );

    if (FAILED(hr)) {
        OutputDebugString(L"[ERROR] Не удалось создать D3D11 устройство\n");
        pAdapter->Release();
        pFactory->Release();
        return hr;
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = m_hWnd;
    swapChainDesc.SampleDesc.Count = 4;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    hr = pFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
    pAdapter->Release();
    pFactory->Release();

    if (FAILED(hr)) {
        OutputDebugString(L"[ERROR] Не удалось создать цепочку подкачки\n");
        return hr;
    }

    if (SUCCEEDED(hr)) hr = ConfigureBackBuffer();
    if (SUCCEEDED(hr)) hr = InitGeometry(init_code);
    if (SUCCEEDED(hr)) hr = InitBufferShader();

    if (FAILED(hr))
    {
        Terminate();
    }

    return hr;
}

HRESULT Render::InitBufferShader()
{
    ID3DBlob* vsBlob = nullptr;
    HRESULT hr = CompileShader(L"VertexColor.vs", &vsBlob);
    if (FAILED(hr)) {
        OutputDebugString(L"[ERROR] Ошибка компиляции вершинного шейдера\n");
        return hr;
    }

    ID3DBlob* psBlob = nullptr;
    hr = CompileShader(L"PixelColor.ps", &psBlob);
    if (FAILED(hr)) {
        vsBlob->Release();
        OutputDebugString(L"[ERROR] Ошибка компиляции пиксельного шейдера\n");
        return hr;
    }

    // Создание шейдеров
    hr = m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_pVertexShader);
    if (FAILED(hr)) {
        OutputDebugString(L"[ERROR] Не удалось создать вершинный шейдер\n");
        vsBlob->Release();
        psBlob->Release();
        return hr;
    }

    hr = m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pPixelShader);
    if (FAILED(hr)) {
        OutputDebugString(L"[ERROR] Не удалось создать пиксельный шейдер\n");
        vsBlob->Release();
        psBlob->Release();
        return hr;
    }

    // Создание входного лэйаута
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    hr = m_pDevice->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pInputLayout);
    vsBlob->Release();
    psBlob->Release();

    if (FAILED(hr)) {
        OutputDebugString(L"[ERROR] Ошибка создания входного лэйаута\n");
    }
    return hr;
}

HRESULT Render::InitGeometry(ModelFactory::ModelCode code) {
    HRESULT hr = InitModel(code);
    if (FAILED(hr)) {
        Terminate();
        return hr;
    }

    hr = InitCamera();
    if (FAILED(hr)) {
        Terminate();
        return hr;
    }

    return hr;
}

HRESULT Render::InitCamera() {
   camera = new Camera(m_pDeviceContext);
   return camera->InitVPBuffer(m_pDevice);
}

HRESULT Render::InitModel(ModelFactory::ModelCode code) {
    m_currentModel = ModelFactory::CreateModel(code, m_pDeviceContext);
    return m_currentModel->InitModel(m_pDevice);
}

void Render::Terminate()
{
    if (m_currentModel) {
        ModelFactory::ReleaseModel(m_currentModel);
        m_currentModel = nullptr;
    }
    if (camera) {
        delete camera;
        camera = nullptr;
    }

    if (m_pDeviceContext) {
        m_pDeviceContext->Release();
        m_pDeviceContext = nullptr;
    }

    if (m_pSwapChain) {
        m_pSwapChain->Release();
        m_pSwapChain = nullptr;
    }

    if (m_pInputLayout) { 
        m_pInputLayout->Release(); 
        m_pInputLayout = nullptr;
    }

    if (m_pVertexShader) {
        m_pVertexShader->Release(); 
        m_pVertexShader = nullptr;
    }
    if (m_pPixelShader) {
        m_pPixelShader->Release(); 
        m_pPixelShader = nullptr;
    };
    if (m_pRenderTargetView) { 
        m_pRenderTargetView->Release(); 
        m_pRenderTargetView = nullptr;
    }

    if (m_pDevice) {
        m_pDevice->Release();
        m_pDevice = nullptr;
    }
}

void Render::SetModel(ModelFactory::ModelCode code) {
    if (m_currentModel) ModelFactory::ReleaseModel(m_currentModel);
    m_currentModel = ModelFactory::CreateModel(code, m_pDeviceContext);
}


HRESULT Render::CompileShader(const std::wstring& path, ID3DBlob** pBlob) {
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* errorBlob = nullptr;
    const D3D_SHADER_MACRO defines[] = { nullptr, nullptr };

    const std::wstring ext = path.substr(path.find_last_of(L".") + 1);
    const std::string target = (ext == L"vs") ? "vs_5_0" : "ps_5_0";

    HRESULT hr = D3DCompileFromFile(
        path.c_str(),
        defines,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        target.c_str(),
        flags,
        0,
        pBlob,
        &errorBlob
    );

    if (FAILED(hr) && errorBlob) {
        std::string errorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
        errorBlob->Release();
    }
    return hr;
}

void Render::UpdateCamera(WPARAM wParam) {
    switch(wParam) {
        case 'W':
            // Upward rotation
            camera->Rotate({ 0.0f, 0.01f });
            break;
        case 'S': // Rotating downwards
            camera->Rotate({ 0.0f, -0.01f });
            break;
        case 'A': // Left rotation
            camera->Rotate({ -0.01f, 0.0f });
            break;
        case 'D': // Right rotation
            camera->Rotate({0.01f, 0.0f});
            break;
        case VK_UP:
            camera->Move({ 0.0f, 1.0f, 0.0f });
            break;
        case VK_DOWN:
            camera->Move({ 0.0f, -1.0f, 0.0f });
            break;
        case VK_LEFT:
            camera->Move({ -1.0f, 0.0f, 0.0f });
            break;
        case VK_RIGHT:
            camera->Move({ 1.0f, 0.0f, 0.0f });
            break;
        case VK_ADD:
        case 0xBB:
            camera->Move({ 0.0f, 0.0f, 1.0f });
            break;

    }

}


void Render::RenderStart()
{
    m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);
    float BackColor[4] = { 0.48f, 0.57f, 0.48f, 1.0f };
    m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, BackColor);
    m_currentModel->Update(0.0);
    RECT rc;
    GetClientRect(m_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    HRESULT hr = camera->CameraUpdate((float)width / (float)height);
    m_pDeviceContext->IASetInputLayout(m_pInputLayout);
    m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
    m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
    m_currentModel->Render();
    m_pSwapChain->Present(1, 0);
}

void Render::SetMVPBuffer()
{
    m_currentModel->Update(0.0);
    RECT rc;
    GetClientRect(m_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    HRESULT hr = camera->CameraUpdate((float)width / (float)height);
    
}

HRESULT Render::ConfigureBackBuffer()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
        return hr;

    hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        return hr;

    return hr;
}

void Render::Resize()
{
    if (m_pRenderTargetView)
    {
        m_pRenderTargetView->Release();
        m_pRenderTargetView = nullptr;
    }

    if (m_pSwapChain)
    {
        HRESULT hr;

        RECT rc;
        GetClientRect(m_hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        hr = m_pSwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        if (FAILED(hr))
        {
            MessageBox(nullptr, L"ResizeBuffers failed.", L"Error", MB_OK);
            return;
        }

        HRESULT resultBack = ConfigureBackBuffer();
        if (FAILED(resultBack))
        {
            MessageBox(nullptr, L"Configure back buffer failed.", L"Error", MB_OK);
            return;
        }

        m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);

        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_pDeviceContext->RSSetViewports(1, &vp);
    }
}