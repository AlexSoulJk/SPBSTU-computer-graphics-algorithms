#include "Triangle.h"
void Triangle::Update(float dt) {
    m_modelMatrix *= DirectX::XMMatrixRotationY(DirectX::XM_PI * dt);
}

void Triangle::Render(ID3D11DeviceContext* context) {
    // Обновление константного буфера
    D3D11_MAPPED_SUBRESOURCE mapped;
    context->Map(m_modelCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &m_modelMatrix, sizeof(DirectX::XMMATRIX));
    context->Unmap(m_modelCBuffer, 0);

    // Отрисовка
    context->VSSetConstantBuffers(0, 1, &m_modelCBuffer);
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    context->Draw(36, 0); // Для куба 36 индексов
}