#ifndef RENDER_H
#define RENDER_H

#include <dxgi.h>
#include <d3d11.h>
#include <string>
#include <d3dcompiler.h>

struct Vertex {
    float x, y, z;
    float color[4];
};

class Render {
public:
    Render(HWND hWnd);
    ~Render();

    HRESULT Init();
    void Terminate();
    void RenderStart();
    void Resize();

    void SetMousePos(int x, int y) { m_mousePos.x = x; m_mousePos.y = y; }
    HWND GetHWND() const { return m_hWnd; }
    IDXGISwapChain* GetSwapChain() const { return m_pSwapChain; }

private:
    HRESULT InitGeometry();
    HRESULT InitShaders();
    HRESULT CompileShader(const std::wstring& path, ID3DBlob** pCode = nullptr);
    HRESULT ConfigureBackBuffer();
    HRESULT InitBlend();

    HWND m_hWnd;
    POINT m_mousePos = { 0, 0 };

    // DirectX resources
    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pDeviceContext = nullptr;
    IDXGISwapChain* m_pSwapChain = nullptr;
    ID3D11RenderTargetView* m_pRenderTargetView = nullptr;

    // Geometry resources
    ID3D11Buffer* m_pVertexBuffer = nullptr;
    ID3D11Buffer* m_pIndexBuffer = nullptr;
    ID3D11InputLayout* m_pInputLayout = nullptr;
    ID3D11VertexShader* m_pVertexShader = nullptr;
    ID3D11PixelShader* m_pPixelShader = nullptr;
    // Фактор для смешивания.
    ID3D11BlendState* m_pBlendState = nullptr;
};

#endif