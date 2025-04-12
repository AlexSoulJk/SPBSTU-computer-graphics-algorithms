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
    void TerminateBufferShader();
    void TerminateSkybox();
    HRESULT InitBufferShader(ModelFactory::ModelCode code);
    HRESULT InitCamera();
    HRESULT InitModel(ModelFactory::ModelCode code);

    HRESULT InitSkybox();
    HRESULT InitTObject();
    void RenderTObject(float aspecto);
    void RenderCubes(float aspecto);
    void RenderSkybox(XMMATRIX proj);
    void TerminateTObject();


    HRESULT CompileShader(const std::wstring& path, ID3D11VertexShader** ppVertexShader, ID3D11PixelShader** ppPixelShader, ID3DBlob** pCodeShader);

    void RenderStart();
    void Resize();
    void UpdateCamera(WPARAM wParam);
    void SetMousePos(int x, int y) { m_mousePos.x = x; m_mousePos.y = y; }
    
    HWND GetHWND() const { return m_hWnd; }
    IDXGISwapChain* GetSwapChain() const { return m_pSwapChain; }
    ~Render() {Terminate();}

private:
    HRESULT ConfigureBackBuffer();
    void SetMVPBuffer(float aspectRatio);
    void RenderCubes(XMMATRIX view, XMMATRIX proj);
    HWND m_hWnd;
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pDeviceContext;

    IDXGISwapChain* m_pSwapChain;
    ID3D11RenderTargetView* m_pRenderTargetView;

    ID3D11PixelShader* m_pPixelShader;
    ID3D11VertexShader* m_pVertexShader;
    ID3D11InputLayout* m_pInputLayout;

    ID3D11DepthStencilView* m_pDepthView;
    ID3D11ShaderResourceView* m_pTextureView;
    ID3D11SamplerState* m_pSamplerState;

    ID3D11VertexShader* m_pSkyboxVS = nullptr;
    ID3D11PixelShader* m_pSkyboxPS = nullptr;
    ID3D11InputLayout* m_pSkyboxLayout = nullptr;
    ID3D11Buffer* m_pSkyboxVB = nullptr;
    ID3D11ShaderResourceView* m_pSkyboxSRV = nullptr;
    ID3D11Buffer* m_pSkyboxVPBuffer = nullptr;

    ID3D11Buffer* m_pColorBuffer = nullptr;
    ID3D11Buffer* m_pTObjectVertexBuffer = nullptr;
    ID3D11Buffer* m_pTObjectIndexBuffer = nullptr;

    ID3D11PixelShader* m_pTObjectPS = nullptr;
    ID3D11VertexShader* m_pTObjectVS = nullptr;
    ID3D11InputLayout* m_pTObjectLayout = nullptr;

    ID3D11BlendState* m_pBlendState;
    ID3D11DepthStencilState* m_pStateTObject = nullptr;;

    float m_CubeAngle = 0.0f;

    WCHAR* m_szTitle;
    WCHAR* m_szWindowClass;
    POINT m_mousePos = { 0, 0 };
    Camera* camera;
    ModelManagerAbstract* m_currentModel;
};
#endif
