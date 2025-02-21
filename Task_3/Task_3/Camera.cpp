#include "Camera.h"
Camera::Camera() :
    position(0, 0, -5),
    pitch(0),
    yaw(0)
{
}

void Camera::Update(float dt) {
    // Обработка ввода
    if (GetAsyncKeyState('W') & 0x8000) position.z += dt;
    if (GetAsyncKeyState('S') & 0x8000) position.z -= dt;
    if (GetAsyncKeyState('A') & 0x8000) yaw -= dt;
    if (GetAsyncKeyState('D') & 0x8000) yaw += dt;
}

DirectX::XMMATRIX Camera::GetViewMatrix() const {
    return DirectX::XMMatrixLookAtLH(
        DirectX::XMLoadFloat3(&position),
        DirectX::XMVectorSet(0, 0, 0, 0),
        DirectX::XMVectorSet(0, 1, 0, 0)
    );
}

DirectX::XMMATRIX Camera::GetProjMatrix() const {
    return DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4, 16.0f / 9.0f, 0.1f, 100.0f
    );
}