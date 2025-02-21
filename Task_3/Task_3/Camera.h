#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>

class Camera {
public:
    Camera();
    void Update(float dt);
    DirectX::XMMATRIX GetViewMatrix() const;
    DirectX::XMMATRIX GetProjMatrix() const;

    DirectX::XMFLOAT3 position;
    float pitch, yaw;
};