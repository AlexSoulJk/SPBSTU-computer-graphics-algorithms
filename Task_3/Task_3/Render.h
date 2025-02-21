#ifndef RENDER_H
#define RENDER_H
#pragma once
#include <dxgi.h>
#include <d3d11.h>
#include <string>
#include <d3dcompiler.h>
#include "Camera.h"
#include "ModelFactory.h"


class Render {
public:
    Render(HWND hWnd);
    ~Render();

    HRESULT Init(ModelFactory::ModelCode code = ModelFactory::ModelCode::cube);
    void Terminate();
    void RenderStart();
    void Resize();
    
    Camera* camera;

    #pragma region GetHWND
    HWND GetHWND() const { return m_hWnd; }
    #pragma endregion

    #pragma region GetSwapChain
    IDXGISwapChain* GetSwapChain() const { return m_pSwapChain; }
    #pragma endregion

    #pragma region SetMousePos
    void SetMousePos(int x, int y) { m_mousePos.x = x; m_mousePos.y = y; }
    #pragma endregion

private:
    HWND m_hWnd;
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

    ModelManagerAbstract* m_currentModel;
    POINT m_mousePos = { 0, 0 };

    void CreateDepthBuffer();
    void SetModel(ModelFactory::ModelCode code);
    HRESULT InitGeometry();
    HRESULT InitShaders();
    HRESULT CompileShader(const std::wstring& path, ID3DBlob** pCode = nullptr);
    HRESULT ConfigureBackBuffer();
    HRESULT InitBlend();

};

#endif