#ifndef RENDER_CLASS_H
#define RENDER_CLASS_H

#include <dxgi.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <iostream>
#include "Camera.h"
#include "ModelFactory.h"

using namespace DirectX;

class Render
{
public:
    Render() = default;
    Render(HWND hwnd);

    HRESULT Init(WCHAR szTitle[], WCHAR szWindowClass[]);
    void Terminate();

    HRESULT InitBufferShader();

    HRESULT InitGeometry(ModelFactory::ModelCode code);
    HRESULT InitCamera();
    HRESULT InitModel(ModelFactory::ModelCode code);


    HRESULT CompileShader(const std::wstring& path, ID3DBlob** pCodeShader=nullptr);

    void RenderStart();
    void Resize();
    void UpdateCamera(WPARAM wParam);
    void SetMousePos(int x, int y) { m_mousePos.x = x; m_mousePos.y = y; }
    
    HWND GetHWND() const { return m_hWnd; }
    IDXGISwapChain* GetSwapChain() const { return m_pSwapChain; }
    ~Render() {Terminate();}

private:
    HRESULT ConfigureBackBuffer();
    void SetMVPBuffer();
    void SetModel(ModelFactory::ModelCode code);
    HWND m_hWnd;
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pDeviceContext;

    IDXGISwapChain* m_pSwapChain;
    ID3D11RenderTargetView* m_pRenderTargetView;

    ID3D11PixelShader* m_pPixelShader;
    ID3D11VertexShader* m_pVertexShader;
    ID3D11InputLayout* m_pInputLayout;

    float m_CubeAngle = 0.0f;

    WCHAR* m_szTitle;
    WCHAR* m_szWindowClass;
    POINT m_mousePos = { 0, 0 };
    Camera* camera;
    ModelManagerAbstract* m_currentModel;
};
#endif
