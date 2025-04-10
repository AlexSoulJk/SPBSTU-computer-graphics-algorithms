#include "framework.h"
#include "Render.h"
#include "DDSTextureLoader11.h"
#include <filesystem>

#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")

struct MatrixBuffer
{
    XMMATRIX m;
};

struct SkyboxVertex
{
    float x, y, z;
};

struct TransparencyObjectVertex {
    float x, y, z;
};

struct ColorBuffer
{
    XMFLOAT4 color;
};

Render::Render(HWND hWnd) : m_hWnd(hWnd), camera(nullptr), m_currentModel(nullptr),
    m_pDevice(nullptr),
    m_pDeviceContext(nullptr),
    m_pSwapChain(nullptr),
    m_pRenderTargetView(nullptr),
    m_pPixelShader(nullptr),
    m_pVertexShader(nullptr),
    m_pInputLayout(nullptr),
    m_pDepthView(nullptr),
    m_pTextureView(nullptr),
    m_pSamplerState(nullptr),
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
    swapChainDesc.SampleDesc.Count = 1;
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
    if (SUCCEEDED(hr)) hr = InitBufferShader(init_code);
    if (SUCCEEDED(hr)) hr = InitCamera();
    if (SUCCEEDED(hr)) hr = InitSkybox();
    if (SUCCEEDED(hr)) hr = InitTObject();

    if (FAILED(hr))
    {
        Terminate();
    }

    return hr;
}

HRESULT Render::InitBufferShader(ModelFactory::ModelCode code)
{
    ID3DBlob* vsBlob = nullptr;
    HRESULT hr = CompileShader(L"VertexColor.vs", &m_pVertexShader, nullptr, &vsBlob);
    if (FAILED(hr)) {
        OutputDebugString(L"[ERROR] Ошибка компиляции вершинного шейдера\n");
        return hr;
    }


    ID3DBlob* psBlob = nullptr;
    hr = CompileShader(L"PixelColor.ps", nullptr, &m_pPixelShader, nullptr);
    
    if (FAILED(hr)) {
        OutputDebugString(L"[ERROR] Ошибка компиляции пиксельного шейдера\n");
        return hr;
    }

    // Создание входного лэйаута
    static const D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    hr = m_pDevice->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pInputLayout);

    if (FAILED(hr)) {
        OutputDebugString(L"[ERROR] Ошибка создания входного лэйаута\n");
        return hr;
    }

    hr = DirectX::CreateDDSTextureFromFile(m_pDevice, L"ava.dds", nullptr, &m_pTextureView);
    if (FAILED(hr))
        return hr;

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = m_pDevice->CreateSamplerState(&sampDesc, &m_pSamplerState);
    if (SUCCEEDED(hr)) hr = InitModel(code);

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

HRESULT Render::InitSkybox()
{
    ID3DBlob* pVertexCode = nullptr;
    ID3DBlob* psBlob = nullptr;
    HRESULT result = CompileShader(L"VertexSkybox.vs", &m_pSkyboxVS, nullptr, &pVertexCode);
    if (SUCCEEDED(result))
    {
        result = CompileShader(L"PixelSkybox.ps", nullptr, &m_pSkyboxPS, &psBlob);
    }

    D3D11_INPUT_ELEMENT_DESC skyboxLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (SUCCEEDED(result))
    {
        result = m_pDevice->CreateInputLayout(skyboxLayout, 1, pVertexCode->GetBufferPointer(), pVertexCode->GetBufferSize(), &m_pSkyboxLayout);
    }

    SkyboxVertex SkyboxVertices[] =
    {
        { -1.0f, -1.0f, -1.0f },
        { -1.0f,  1.0f, -1.0f },
        {  1.0f,  1.0f, -1.0f },
        { -1.0f, -1.0f, -1.0f },
        {  1.0f,  1.0f, -1.0f },
        {  1.0f, -1.0f, -1.0f },

        {  1.0f, -1.0f,  1.0f },
        {  1.0f,  1.0f,  1.0f },
        { -1.0f,  1.0f,  1.0f },
        {  1.0f, -1.0f,  1.0f },
        { -1.0f,  1.0f,  1.0f },
        { -1.0f, -1.0f,  1.0f },

        { -1.0f, -1.0f,  1.0f },
        { -1.0f,  1.0f,  1.0f },
        { -1.0f,  1.0f, -1.0f },
        { -1.0f, -1.0f,  1.0f },
        { -1.0f,  1.0f, -1.0f },
        { -1.0f, -1.0f, -1.0f },

        { 1.0f, -1.0f, -1.0f },
        { 1.0f,  1.0f, -1.0f },
        { 1.0f,  1.0f,  1.0f },
        { 1.0f, -1.0f, -1.0f },
        { 1.0f,  1.0f,  1.0f },
        { 1.0f, -1.0f,  1.0f },

        { -1.0f, 1.0f, -1.0f },
        { -1.0f, 1.0f,  1.0f },
        { 1.0f, 1.0f,  1.0f },
        { -1.0f, 1.0f, -1.0f },
        { 1.0f, 1.0f,  1.0f },
        { 1.0f, 1.0f, -1.0f },

        { -1.0f, -1.0f,  1.0f },
        { -1.0f, -1.0f, -1.0f },
        { 1.0f, -1.0f, -1.0f },
        { -1.0f, -1.0f,  1.0f },
        { 1.0f, -1.0f, -1.0f },
        { 1.0f, -1.0f,  1.0f },
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SkyboxVertex) * ARRAYSIZE(SkyboxVertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = SkyboxVertices;
    result = m_pDevice->CreateBuffer(&bd, &initData, &m_pSkyboxVB);
    if (FAILED(result))
        return result;

    D3D11_BUFFER_DESC vpBufferDesc = {};
    vpBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vpBufferDesc.ByteWidth = sizeof(XMMATRIX);
    vpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    vpBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    result = m_pDevice->CreateBuffer(&vpBufferDesc, nullptr, &m_pSkyboxVPBuffer);
    if (FAILED(result))
        return result;

    result = CreateDDSTextureFromFile(m_pDevice, L"skybox.dds", nullptr, &m_pSkyboxSRV);
    if (FAILED(result))
        return result;

    return S_OK;
}

HRESULT Render::InitTObject() {
    // Создание вершинного буфера
    ID3DBlob* pVertexCode = nullptr;
    ID3DBlob* psBlob = nullptr;
    HRESULT result = CompileShader(L"VertexForTransparencyObject.vs", &m_pTObjectVS, nullptr, &pVertexCode);
    if (SUCCEEDED(result))
    {
        result = CompileShader(L"PixelForTransparencyObject.ps", nullptr, &m_pTObjectPS, &psBlob);
    }

    D3D11_INPUT_ELEMENT_DESC TObjectLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (SUCCEEDED(result))
    {
        result = m_pDevice->CreateInputLayout(TObjectLayout, 1, pVertexCode->GetBufferPointer(), pVertexCode->GetBufferSize(), &m_pTObjectLayout);
    }

    TransparencyObjectVertex TObjectVertices[] =
    {
        {-0.75, -0.75, 0.0},
        {-0.75,  0.75, 0.0},
        { 0.75,  0.75, 0.0},
        { 0.75, -0.75, 0.0}
    };

    WORD TObjectIndices[] =
    {
        0, 1, 2,
        0, 2, 3
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(TransparencyObjectVertex) * ARRAYSIZE(TObjectVertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = TObjectVertices;
    result = m_pDevice->CreateBuffer(&bd, &initData, &m_pTObjectVertexBuffer);
    if (FAILED(result))
        return result;

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(TObjectIndices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    initData.pSysMem = TObjectIndices;
    result = m_pDevice->CreateBuffer(&bd, &initData, &m_pTObjectIndexBuffer);
    if (FAILED(result))
        return result;

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ColorBuffer);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;
    result = m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_pColorBuffer);
    if (FAILED(result))
        return result;

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    result = m_pDevice->CreateBlendState(&blendDesc, &m_pBlendState);
    if (FAILED(result))
        return result;

    D3D11_DEPTH_STENCIL_DESC dsDescTrans = {};
    dsDescTrans.DepthEnable = true;
    dsDescTrans.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDescTrans.DepthFunc = D3D11_COMPARISON_LESS;
    result = m_pDevice->CreateDepthStencilState(&dsDescTrans, &m_pStateTObject);
    if (FAILED(result))
        return result;
}

void Render::TerminateBufferShader()
{
    if (m_currentModel) {
        ModelFactory::ReleaseModel(m_currentModel);
        m_currentModel = nullptr;
    }

    if (camera) {
        delete camera;
        camera = nullptr;
    }
    if (m_pInputLayout) {
        m_pInputLayout->Release();
        m_pInputLayout = nullptr;
    }


    if (m_pPixelShader) {
        m_pPixelShader->Release();
        m_pPixelShader = nullptr;
    }


    if (m_pVertexShader) {
        m_pVertexShader->Release();
        m_pVertexShader = nullptr;
    }


    if (m_pTextureView) {
        m_pTextureView->Release();
        m_pTextureView = nullptr;
    }


    if (m_pSamplerState) {
        m_pSamplerState->Release();
        m_pSamplerState = nullptr;
    }
        
}

void Render::TerminateSkybox()
{
    if (m_pSkyboxVB) {
        m_pSkyboxVB->Release();
        m_pSkyboxVB = nullptr;
    }

    if (m_pSkyboxSRV) {
        m_pSkyboxSRV->Release();
        m_pSkyboxSRV = nullptr;
    }

    if (m_pSkyboxVPBuffer) {
        m_pSkyboxVPBuffer->Release();
        m_pSkyboxVPBuffer = nullptr;
    }

    if (m_pSkyboxLayout) {
        m_pSkyboxLayout->Release();
        m_pSkyboxLayout = nullptr;
    }

    if (m_pSkyboxVS) {
        m_pSkyboxVS->Release();
        m_pSkyboxVS = nullptr;
    }

    if (m_pSkyboxPS) {
        m_pSkyboxPS->Release();
        m_pSkyboxPS = nullptr;
    }
}

void Render::TerminateTObject()
{
    if (m_pTObjectVertexBuffer) {
        m_pTObjectVertexBuffer->Release();
        m_pTObjectVertexBuffer = nullptr;
    }
        

    if (m_pTObjectIndexBuffer) {
        m_pTObjectIndexBuffer->Release();
        m_pTObjectIndexBuffer = nullptr;
    }
        

    if (m_pTObjectPS) {
        m_pTObjectPS->Release();
        m_pTObjectPS = nullptr;
    }
        

    if (m_pTObjectVS) {
        m_pTObjectVS->Release();
        m_pTObjectVS = nullptr;
    }
        

    if (m_pTObjectLayout) {
        m_pTObjectLayout->Release();
        m_pTObjectLayout = nullptr;
    }
        

    if (m_pColorBuffer) {
        m_pColorBuffer->Release();
        m_pColorBuffer = nullptr;
    }
        

    if (m_pBlendState) {
        m_pBlendState->Release();
        m_pBlendState = nullptr;
    }
        

    if (m_pStateTObject) {
        m_pStateTObject->Release();
        m_pStateTObject = nullptr;
    }
        
}

void Render::Terminate()
{
    TerminateBufferShader();
    TerminateSkybox();
    TerminateTObject();

    if (m_pDeviceContext) {
        m_pDeviceContext->ClearState();
        m_pDeviceContext->Release();
        m_pDeviceContext = nullptr;
    }

    if (m_pRenderTargetView)
    {
        m_pRenderTargetView->Release();
        m_pRenderTargetView = nullptr;
    }

    if (m_pDepthView)
    {
        m_pDepthView->Release();
        m_pDepthView = nullptr;
    }

    if (m_pSwapChain) {
        m_pSwapChain->Release();
        m_pSwapChain = nullptr;
    }

    if (m_pDevice) {
        m_pDevice->Release();
        m_pDevice = nullptr;
    }
}


std::wstring Extension(const std::wstring& path)
{
    size_t dotPos = path.find_last_of(L".");
    if (dotPos == std::wstring::npos || dotPos == 0)
    {
        return L"";
    }
    return path.substr(dotPos + 1);
}

HRESULT Render::CompileShader(const std::wstring& path, ID3D11VertexShader** ppVertexShader, ID3D11PixelShader** ppPixelShader, ID3DBlob** pCodeShader)
{
    std::wstring extension = Extension(path);

    std::string platform = "";

    if (extension == L"vs")
    {
        platform = "vs_5_0";
    }
    else if (extension == L"ps")
    {
        platform = "ps_5_0";
    }

    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif 

    ID3DBlob* pCode = nullptr;
    ID3DBlob* pErr = nullptr;

    HRESULT result = D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", platform.c_str(), 0, 0, &pCode, &pErr);
    if (!SUCCEEDED(result) && pErr != nullptr)
    {
        OutputDebugStringA((const char*)pErr->GetBufferPointer());
    }
    if (pErr)
        pErr->Release();

    if (SUCCEEDED(result))
    {
        if (extension == L"vs" && ppVertexShader)
        {
            result = m_pDevice->CreateVertexShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), nullptr, ppVertexShader);
            if (FAILED(result))
            {
                pCode->Release();
                return result;
            }
        }
        else if (extension == L"ps" && ppPixelShader)
        {
            result = m_pDevice->CreatePixelShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), nullptr, ppPixelShader);
            if (FAILED(result))
            {
                pCode->Release();
                return result;
            }
        }
    }

    if (pCodeShader)
    {
        *pCodeShader = pCode;
    }
    else
    {
        pCode->Release();
    }
    return result;
}

void Render::UpdateCamera(WPARAM wParam) {
    switch(wParam) {
        case 'W':
            camera->Rotate({ 0.0f, 0.01f });
            break;
        case 'S':
            camera->Rotate({ 0.0f, -0.01f });
            break;
        case 'A':
            camera->Rotate({ -0.01f, 0.0f });
            break;
        case 'D':
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
            camera->Move({ 0.0f, 0.0f, 1.0f });
            break;
        case VK_SUBTRACT:
            camera->Move({ 0.0f, 0.0f, -1.0f });
            break;

    }

}

void Render::RenderStart()
{
    m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthView);
    float BackColor[4] = { 0.48f, 0.57f, 0.48f, 1.0f };
    m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, BackColor);
    m_pDeviceContext->ClearDepthStencilView(m_pDepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    

    RECT rc;
    GetClientRect(m_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    float aspect = (float)width / (float)height;
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.1f, 100.0f);

    
    RenderSkybox(proj);
    RenderCubes(aspect);

    RenderTObject(aspect);

    m_pSwapChain->Present(1, 0);
}

void Render::RenderSkybox(XMMATRIX proj)
{
    XMMATRIX rotLRSky = XMMatrixRotationY(-camera->LRAngle);
    XMMATRIX rotUDSky = XMMatrixRotationX(-camera->UDAngle);
    XMMATRIX viewSkybox = rotLRSky * rotUDSky;
    XMMATRIX vpSkybox = XMMatrixTranspose(viewSkybox * proj);

    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(m_pDeviceContext->Map(m_pSkyboxVPBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        memcpy(mapped.pData, &vpSkybox, sizeof(XMMATRIX));
        m_pDeviceContext->Unmap(m_pSkyboxVPBuffer, 0);
    }

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    ID3D11DepthStencilState* pDSStateSkybox = nullptr;
    m_pDevice->CreateDepthStencilState(&dsDesc, &pDSStateSkybox);
    m_pDeviceContext->OMSetDepthStencilState(pDSStateSkybox, 0);

    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_FRONT;
    rsDesc.FrontCounterClockwise = false;
    ID3D11RasterizerState* pSkyboxRS = nullptr;
    if (SUCCEEDED(m_pDevice->CreateRasterizerState(&rsDesc, &pSkyboxRS)))
    {
        m_pDeviceContext->RSSetState(pSkyboxRS);
    }

    UINT stride = sizeof(SkyboxVertex);
    UINT offset = 0;
    m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pSkyboxVB, &stride, &offset);
    m_pDeviceContext->IASetInputLayout(m_pSkyboxLayout);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_pDeviceContext->VSSetShader(m_pSkyboxVS, nullptr, 0);
    m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pSkyboxVPBuffer);

    m_pDeviceContext->PSSetShader(m_pSkyboxPS, nullptr, 0);
    m_pDeviceContext->PSSetShaderResources(0, 1, &m_pSkyboxSRV);
    m_pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerState);

    m_pDeviceContext->Draw(36, 0);

    pDSStateSkybox->Release();
    if (pSkyboxRS)
    {
        pSkyboxRS->Release();
        m_pDeviceContext->RSSetState(nullptr);
    }
}

void Render::RenderCubes(float aspecto)
{
    m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthView);
    m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);

    m_currentModel->Update(0.0);

    camera->CameraUpdate(aspecto);

    UINT stride = sizeof(ModelManagerAbstract::Vertex);
    UINT offset = 0;

    m_pDeviceContext->IASetVertexBuffers(0, 1, &(m_currentModel->m_pVertexBuffer), &stride, &offset);
    m_pDeviceContext->IASetIndexBuffer(m_currentModel->m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pDeviceContext->IASetInputLayout(m_pInputLayout);

    m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
    m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

    m_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureView);
    m_pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerState);
    m_pDeviceContext->DrawIndexed(36, 0, 0);


    static float orbit = 0.0f;
    orbit += 0.01f;
    XMMATRIX model2 = XMMatrixRotationY(orbit) * XMMatrixTranslation(0.0f, 0.0f, 5.0f) * XMMatrixRotationY(-orbit);
    XMMATRIX mT2 = XMMatrixTranspose(model2);
    m_pDeviceContext->UpdateSubresource((m_currentModel->m_pModelBuffer), 0, nullptr, &mT2, 0, 0);
    m_pDeviceContext->DrawIndexed(36, 0, 0);
}

void Render::RenderTObject(float aspecto)
{
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.FrontCounterClockwise = false;

    ID3D11RasterizerState* pRSState = nullptr;
    m_pDevice->CreateRasterizerState(&rsDesc, &pRSState);
    m_pDeviceContext->RSSetState(pRSState);

    m_pDeviceContext->OMSetDepthStencilState(m_pStateTObject, 0);
    m_pDeviceContext->OMSetBlendState(m_pBlendState, nullptr, 0xFFFFFFFF);


    UINT stride = sizeof(TransparencyObjectVertex);
    UINT offset = 0;
    m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pTObjectVertexBuffer, &stride, &offset);
    m_pDeviceContext->IASetIndexBuffer(m_pTObjectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pDeviceContext->IASetInputLayout(m_pTObjectLayout);

    m_pDeviceContext->VSSetShader(m_pTObjectVS, nullptr, 0);
    m_pDeviceContext->VSSetConstantBuffers(0, 1, &(m_currentModel->m_pModelBuffer));
    m_pDeviceContext->VSSetConstantBuffers(1, 1, &(camera->m_pVPBuffer));

    m_pDeviceContext->PSSetShader(m_pTObjectPS, nullptr, 0);
    m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pColorBuffer);

    static float angle = 0.0f;
    angle += 0.015f;

    XMMATRIX modelTObjectRed = XMMatrixTranslation(sinf(angle) * 2.0f, 1.0f, -2.0f);
    XMMATRIX mTTObjectRed = XMMatrixTranspose(modelTObjectRed);
    XMFLOAT4 redColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f);

    XMMATRIX modelTObjectGreen = XMMatrixTranslation(-sinf(angle) * 2.0f, 1.0f, -3.0f);
    XMMATRIX mTTObjectGreen = XMMatrixTranspose(modelTObjectGreen);
    XMFLOAT4 greenColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.5f);

    XMFLOAT3 redObjectPosition;
    XMStoreFloat3(&redObjectPosition, modelTObjectRed.r[3]);

    XMFLOAT3 greenObjectPosition;
    XMStoreFloat3(&greenObjectPosition, modelTObjectGreen.r[3]);

    XMMATRIX viewMatrix = camera->GetViewMatrix();
    XMVECTOR cameraLookDirection = XMVector3Normalize(viewMatrix.r[2]);

    // Позиция камеры
    XMVECTOR cameraPos = XMLoadFloat3(&(camera->position));

    // Векторы от камеры до объектов
    XMVECTOR toRedObject = XMLoadFloat3(&redObjectPosition) - cameraPos;
    XMVECTOR toGreenObject = XMLoadFloat3(&greenObjectPosition) - cameraPos;

    // Проекции на вектор взгляда
    float redProjection = XMVectorGetX(XMVector3Dot(toRedObject, cameraLookDirection));
    float greenProjection = XMVectorGetX(XMVector3Dot(toGreenObject, cameraLookDirection));

    // Сортируем объекты по проекции
    if (redProjection >= greenProjection)
    {
        // Рисуем красный объект, затем зелёный
        m_pDeviceContext->UpdateSubresource((m_currentModel->m_pModelBuffer), 0, nullptr, &mTTObjectRed, 0, 0);
        m_pDeviceContext->UpdateSubresource(m_pColorBuffer, 0, nullptr, &redColor, 0, 0);
        m_pDeviceContext->DrawIndexed(6, 0, 0);

        m_pDeviceContext->UpdateSubresource((m_currentModel->m_pModelBuffer), 0, nullptr, &mTTObjectGreen, 0, 0);
        m_pDeviceContext->UpdateSubresource(m_pColorBuffer, 0, nullptr, &greenColor, 0, 0);
        m_pDeviceContext->DrawIndexed(6, 0, 0);
    }
    else
    {
        // Рисуем зелёный объект, затем красный
        m_pDeviceContext->UpdateSubresource((m_currentModel->m_pModelBuffer), 0, nullptr, &mTTObjectGreen, 0, 0);
        m_pDeviceContext->UpdateSubresource(m_pColorBuffer, 0, nullptr, &greenColor, 0, 0);
        m_pDeviceContext->DrawIndexed(6, 0, 0);

        m_pDeviceContext->UpdateSubresource((m_currentModel->m_pModelBuffer), 0, nullptr, &mTTObjectRed, 0, 0);
        m_pDeviceContext->UpdateSubresource(m_pColorBuffer, 0, nullptr, &redColor, 0, 0);
        m_pDeviceContext->DrawIndexed(6, 0, 0);
    }

    pRSState->Release();
}


HRESULT Render::ConfigureBackBuffer()
{
    RECT rc;
    GetClientRect(m_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr)) {
        OutputDebugString(L"[ERROR] Не удалось создать pBackBuffer\n");
        return hr;
    }

    hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        return hr;

    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ID3D11Texture2D* pDepthStencil = nullptr;
    hr = m_pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);
    if (FAILED(hr))
        return hr;

    hr = m_pDevice->CreateDepthStencilView(pDepthStencil, nullptr, &m_pDepthView);
    pDepthStencil->Release();
    if (FAILED(hr))
        return hr;

    return hr;
}

void Render::Resize()
{
    // Освободите старые ресурсы
    if (m_pRenderTargetView) {
        m_pRenderTargetView->Release();
        m_pRenderTargetView = nullptr;
    }
    if (m_pDepthView) {
        m_pDepthView->Release();
        m_pDepthView = nullptr;
    }

    if (m_pSwapChain)
    {
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        HRESULT hr = m_pSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"ResizeBuffers failed.", L"Error", MB_OK);
            return;
        }

        if (FAILED(ConfigureBackBuffer())) {
            MessageBox(nullptr, L"Configure back buffer failed.", L"Error", MB_OK);
            return;
        }

        m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthView);

        D3D11_VIEWPORT vp = {};
        vp.Width = static_cast<FLOAT>(width);
        vp.Height = static_cast<FLOAT>(height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_pDeviceContext->RSSetViewports(1, &vp);
    }

}

