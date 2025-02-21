#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
class ModelManagerAbstract
{
public:

    struct Vertex {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
    };
    DirectX::XMMATRIX GetModelMatrix() const { return m_modelMatrix; }
    ModelManagerAbstract(ID3D11Device* device);
    virtual ~ModelManagerAbstract() {
        if (m_vertexBuffer) m_vertexBuffer->Release();
        if (m_indexBuffer) m_indexBuffer->Release();
        if (m_modelCBuffer) m_modelCBuffer->Release();
    }
    virtual void Update(float dt) = 0;
    virtual void Render(ID3D11DeviceContext* context) = 0;
protected:
    ID3D11Device* m_device;
    ID3D11Buffer* m_vertexBuffer = nullptr;
    ID3D11Buffer* m_indexBuffer = nullptr;
    ID3D11Buffer* m_modelCBuffer = nullptr;

    DirectX::XMMATRIX m_modelMatrix = DirectX::XMMatrixIdentity();
    float m_rotationAngle = 0.0f;
};

