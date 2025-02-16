#include "Render.h"
#include <windowsx.h>
#include <iostream>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

Render::Render(HWND hWnd) : m_hWnd(hWnd) {}

Render::~Render() {
    Terminate();
}

HRESULT Render::Init() {
    HRESULT hr = S_OK;

    // Factory creation
    IDXGIFactory* pFactory = nullptr;
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
    if (FAILED(hr)) return hr;

    // Adapter selection
    IDXGIAdapter* pAdapter = nullptr;
    hr = pFactory->EnumAdapters(0, &pAdapter);
    if (FAILED(hr)) { pFactory->Release(); return hr; }

    // Device creation
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
    if (FAILED(hr)) { pAdapter->Release(); pFactory->Release(); return hr; }

    // Swap chain creation
    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 2;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = m_hWnd;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.Windowed = TRUE;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    hr = pFactory->CreateSwapChain(m_pDevice, &swapDesc, &m_pSwapChain);
    pAdapter->Release();
    pFactory->Release();
    if (FAILED(hr)) return hr;

    // Initialization steps
    if (SUCCEEDED(hr)) hr = ConfigureBackBuffer();
    if (SUCCEEDED(hr)) hr = InitGeometry();
    if (SUCCEEDED(hr)) hr = InitShaders();

    return hr;
}

void Render::Terminate() {
    if (m_pInputLayout) { m_pInputLayout->Release(); m_pInputLayout = nullptr; }
    if (m_pVertexShader) { m_pVertexShader->Release(); m_pVertexShader = nullptr; }
    if (m_pPixelShader) { m_pPixelShader->Release(); m_pPixelShader = nullptr; }
    if (m_pVertexBuffer) { m_pVertexBuffer->Release(); m_pVertexBuffer = nullptr; }
    if (m_pIndexBuffer) { m_pIndexBuffer->Release(); m_pIndexBuffer = nullptr; }
    if (m_pRenderTargetView) { m_pRenderTargetView->Release(); m_pRenderTargetView = nullptr; }
    if (m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = nullptr; }
    if (m_pDeviceContext) { m_pDeviceContext->Release(); m_pDeviceContext = nullptr; }
    if (m_pDevice) { m_pDevice->Release(); m_pDevice = nullptr; }
}

HRESULT Render::InitGeometry() {
    // Vertex data
    const Vertex vertices[] = {
    { 0.0f,  0.5f, 0.0f, {1.0f, 0.0f, 0.0f, 1.0f} }, // Красный
    { 0.5f, -0.5f, 0.0f, {0.0f, 1.0f, 0.0f, 1.0f} }, // Зеленый
    {-0.5f, -0.5f, 0.0f, {0.0f, 0.0f, 1.0f, 1.0f} }  // Синий
    };

    // Create vertex buffer
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;

    HRESULT hr = m_pDevice->CreateBuffer(&vbDesc, &vbData, &m_pVertexBuffer);
    if (FAILED(hr)) {
        OutputDebugString(L"Ошибка при создании буфера вершин\n");
        return hr;
    }
}

HRESULT Render::InitShaders() {
    // Compile shaders
    ID3DBlob* vsBlob = nullptr;
    HRESULT hr = CompileShader(L"VertexColor.vs", &vsBlob);
    if (FAILED(hr)) return hr;

    ID3DBlob* psBlob = nullptr;
    hr = CompileShader(L"PixelColor.ps", &psBlob);
    if (FAILED(hr)) { vsBlob->Release(); return hr; }

    // Create shaders
    hr = m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_pVertexShader);
    if (SUCCEEDED(hr)) {
        hr = m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pPixelShader);
    }

    // Create input layout
    if (SUCCEEDED(hr)) {
        D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        hr = m_pDevice->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pInputLayout);
    }

    vsBlob->Release();
    psBlob->Release();
    return hr;
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
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
    }
    return hr;
}

void Render::RenderStart() {
    // Clear back buffer
    RECT rc;
    GetClientRect(m_hWnd, &rc);
    float nx = static_cast<float>(m_mousePos.x) / (rc.right - rc.left);
    float ny = static_cast<float>(m_mousePos.y) / (rc.bottom - rc.top);

    float BackColor[4] = {
        nx,          // Красный зависит от X
        ny,          // Зеленый зависит от Y
        (nx + ny) / 2, // Синий - среднее
        1.0f
    };
    m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, BackColor);

    // Set pipeline state
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pDeviceContext->IASetInputLayout(m_pInputLayout);
    m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
    m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

    // Draw triangle
    m_pDeviceContext->Draw(3, 0);

    // Present
    m_pSwapChain->Present(0, 0);
}

HRESULT Render::ConfigureBackBuffer() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    if (SUCCEEDED(hr)) {
        hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
        pBackBuffer->Release();

        if (SUCCEEDED(hr)) {
            m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);
            
        }
    }
    return hr;
}

void Render::Resize() {
    if (!m_pSwapChain) return;

    m_pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    
    if (m_pRenderTargetView) m_pRenderTargetView->Release();

    RECT rc;
    GetClientRect(m_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    HRESULT hr = m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (SUCCEEDED(hr)) {
        ConfigureBackBuffer();
    }

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<FLOAT>(width);
    vp.Height = static_cast<FLOAT>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pDeviceContext->RSSetViewports(1, &vp);
}