#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace DirectX;
struct CameraBuffer
{
    DirectX::XMMATRIX vp;
};

class Camera {


public:
    Camera();
    Camera(ID3D11DeviceContext* context);
    ~Camera() {
        if (m_pVPBuffer) {
            m_pVPBuffer->Release();
            m_pVPBuffer = nullptr;
        }
    }

    void Move(XMFLOAT3 direction);
    void Rotate(XMFLOAT2 angleDirection);

    HRESULT CameraUpdate(float aspectRatio);
    HRESULT InitVPBuffer(ID3D11Device* device);
    XMMATRIX GetVP(float aspectRatio) const;
    XMMATRIX GetViewMatrix() const;
    XMFLOAT3 position;
    float speed;
    float LRAngle; 
    float UDAngle;
    ID3D11Buffer* m_pVPBuffer;
private:
    ID3D11DeviceContext* m_pDeviceContext;
    
};