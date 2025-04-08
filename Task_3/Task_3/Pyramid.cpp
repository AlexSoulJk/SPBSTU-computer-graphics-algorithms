#include "Pyramid.h"

Pyramid::Pyramid(ID3D11DeviceContext* context) 
    : ModelManagerAbstract(context) {}

HRESULT Pyramid::InitModel(ID3D11Device* device) {
    HRESULT hr;

    // Вершины тетраэдра
    static const Vertex vertices[] = {
        { { 0.0f, 0.0f, 0.0f }, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // 0: Основание 1
        { { 2.0f, 0.0f, 0.0f }, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }, // 1: Основание 2
        { { 1.0f, 0.0f, 2.0f }, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }, // 2: Основание 3
        { { 1.0f, 2.0f, 1.0f }, XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) }  // 3: Вершина
    };

    // Индексы для 4 граней
    WORD indices[] = {
        0, 1, 3,  // Передняя грань
        1, 2, 3,  // Правая грань
        2, 0, 3,   // Левая грань
        0, 1, 2,  // Основание
    };

    // Создание вершинного буфера
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    
    D3D11_SUBRESOURCE_DATA vinit = { vertices };
    hr = device->CreateBuffer(&vbd, &vinit, &m_pVertexBuffer);
    if (FAILED(hr)) return hr;

    // Создание индексного буфера
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    
    D3D11_SUBRESOURCE_DATA iinit = { indices };
    hr = device->CreateBuffer(&ibd, &iinit, &m_pIndexBuffer);
    if (FAILED(hr)) return hr;

    // Константный буфер модели
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(XMMATRIX);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    return device->CreateBuffer(&cbd, nullptr, &m_pModelBuffer);
}

void Pyramid::Update(float dt) {
    m_rotationAngle += 0.5f * dt;
    if (m_rotationAngle > XM_2PI) m_rotationAngle -= XM_2PI;

    XMMATRIX model = XMMatrixRotationY(m_rotationAngle);
    XMMATRIX transposed = XMMatrixTranspose(model);
    
    m_context->UpdateSubresource(m_pModelBuffer, 0, nullptr, &transposed, 0, 0);
    m_context->VSSetConstantBuffers(0, 1, &m_pModelBuffer);
}

void Pyramid::Render() {
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    
    m_context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
    m_context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // 4 грани × 3 вершины = 12 индексов
    m_context->DrawIndexed(12, 0, 0);
}