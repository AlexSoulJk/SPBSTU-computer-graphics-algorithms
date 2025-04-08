#include "Camera.h"

Camera::Camera() :
    position(0, 0, -5.0f),
    speed(0.1f),
    LRAngle(0.0f),
    UDAngle(0.0f),
    m_pDeviceContext(nullptr)
{
}

Camera::Camera(ID3D11DeviceContext* context) : Camera() {
    m_pDeviceContext = context;
};

HRESULT Camera::CameraUpdate(float aspectRatio) {
    XMMATRIX vp = GetVP(aspectRatio);
    XMMATRIX vpT = XMMatrixTranspose(vp);
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = m_pDeviceContext->Map(m_pVPBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        memcpy(mappedResource.pData, &vpT, sizeof(XMMATRIX));
        m_pDeviceContext->Unmap(m_pVPBuffer, 0);
    }
    m_pDeviceContext->VSSetConstantBuffers(1, 1, &m_pVPBuffer);
    return hr;
}

void Camera::Move(XMFLOAT3 direction) {
    position.x += direction.x * speed;
    position.y += direction.y * speed;
    position.z += direction.z * speed;
};

void Camera::Rotate(XMFLOAT2 angleDirection) {
    LRAngle += angleDirection.x;
    UDAngle += angleDirection.y;

    if (LRAngle > XM_2PI) LRAngle -= XM_2PI;
    if (LRAngle < -XM_2PI) LRAngle += XM_2PI;

    if (UDAngle > XM_PIDIV2) UDAngle = XM_PIDIV2;
    if (UDAngle < -XM_PIDIV2) UDAngle = -XM_PIDIV2;
};

XMMATRIX Camera::GetVP(float aspectRatio) const
{
    // ��������� ����������� ������� ������
    XMVECTOR direction = XMVectorSet(
        cosf(UDAngle) * sinf(LRAngle),
        sinf(UDAngle),
        cosf(UDAngle) * cosf(LRAngle),
        0.0f
    );

    // ������� ������
    XMVECTOR eyePos = XMLoadFloat3(&position);

    // �����, � ������� ������� ������ (�����)
    XMVECTOR focusPoint = XMVectorAdd(eyePos, direction);

    // ������ "�����" ��� ������
    static const XMVECTOR upDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // ������� ���� (view matrix)
    XMMATRIX view = XMMatrixLookAtLH(eyePos, focusPoint, upDir);

    // ������� �������� (projection matrix)
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 100.0f);

    // ���������� ������������ ������ ���� � ��������
    return view * proj;
}
HRESULT Camera::InitVPBuffer(ID3D11Device* device) {
    D3D11_BUFFER_DESC vpBufferDesc = {};
    vpBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vpBufferDesc.ByteWidth = sizeof(XMMATRIX);
    vpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    vpBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    return device->CreateBuffer(&vpBufferDesc, nullptr, &m_pVPBuffer);
}
