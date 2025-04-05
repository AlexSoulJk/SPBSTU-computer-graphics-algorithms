#pragma once
#include "Cube.h"

void Cube::Update(float dt) {
    m_CubeAngle += 0.01f;
    if (m_CubeAngle > XM_2PI) m_CubeAngle -= XM_2PI;
    XMMATRIX model = XMMatrixRotationY(m_CubeAngle);
    XMMATRIX mT = XMMatrixTranspose(model);
    m_context->UpdateSubresource(m_pModelBuffer, 0, nullptr, &mT, 0, 0);
    m_context->VSSetConstantBuffers(0, 1, &m_pModelBuffer);
}

void Cube::Render() {
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
    m_context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_context->DrawIndexed(36, 0, 0);
}

HRESULT Cube::InitModel(ID3D11Device* device) {
    HRESULT hr;
    static const Vertex vertices[] =
    {
        { { -1.0f,  1.0f, -1.0f}, DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }, // Синий
        { {  1.0f,  1.0f, -1.0f}, DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }, // Зеленый
        { {  1.0f,  1.0f,  1.0f}, DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }, // Голубой
        { { -1.0f,  1.0f,  1.0f}, DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // Красный
        { { -1.0f, -1.0f, -1.0f}, DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) }, // Фиолетовый
        { {  1.0f, -1.0f, -1.0f}, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) }, // Желтый
        { {  1.0f, -1.0f,  1.0f}, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }, // Белый
        { { -1.0f, -1.0f,  1.0f}, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }  // Черный
    };

    WORD indices[] =
    {
        3,1,0,
        2,1,3,

        0,5,4,
        1,5,0,

        3,4,7,
        0,4,3,

        1,6,5,
        2,6,1,

        2,7,6,
        3,7,2,

        6,4,5,
        7,4,6,
    };

    // Создание буферов
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;
    hr = device->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
    if (FAILED(hr))
        return hr;

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    initData.pSysMem = indices;
    hr = device->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
    if (FAILED(hr))
        return hr;

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(XMMATRIX);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    return device->CreateBuffer(&bd, nullptr, &m_pModelBuffer);
}