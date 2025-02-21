#include "ModelManagerAbstract.h"

ModelManagerAbstract::ModelManagerAbstract(ID3D11Device* device) : m_device(device) {
    // Создание константного буфера
    D3D11_BUFFER_DESC cbDesc = { 0 };
    cbDesc.ByteWidth = sizeof(DirectX::XMMATRIX);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&cbDesc, nullptr, &m_modelCBuffer);
}