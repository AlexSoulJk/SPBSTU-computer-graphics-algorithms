#ifndef RENDER_H
#define RENDER_H

#include <dxgi.h>
#include <d3d11.h>

class Render
{
public:
    Render(HWND hWnd) :m_hWnd(hWnd), m_pDevice(nullptr),
        m_pDeviceContext(nullptr),
        m_pSwapChain(nullptr),
        m_pRenderTargetView(nullptr)
    {}

    HRESULT Init();
    void Terminate();

    void RenderStart();
    void Resize();

    void SetMousePos(int x, int y) { m_mousePos.x = x; m_mousePos.y = y; }
    HWND GetHWND() { return m_hWnd; }
    IDXGISwapChain* GetSwapChain() const { return m_pSwapChain; }

private:
    POINT m_mousePos = { 0, 0 };
    HRESULT ConfigureBackBuffer();
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pDeviceContext;
    IDXGISwapChain* m_pSwapChain;
    ID3D11RenderTargetView* m_pRenderTargetView;
    HWND m_hWnd;
};
#endif