#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
using namespace DirectX;
class ModelManagerAbstract
{
public:

    struct Vertex {
        XMFLOAT3 xyz;
        XMFLOAT3 normal;
        XMFLOAT2 uv;
    };
    ModelManagerAbstract(ID3D11DeviceContext* context);
    virtual ~ModelManagerAbstract() {
        if (m_pVertexBuffer) {
            m_pVertexBuffer->Release(); m_pVertexBuffer = nullptr;
        }
        if (m_pIndexBuffer) {
            m_pIndexBuffer->Release(); m_pIndexBuffer = nullptr;
        }
        if (m_pModelBuffer) {
            m_pModelBuffer->Release(); m_pIndexBuffer = nullptr;
        }
    }
    virtual HRESULT InitModel(ID3D11Device* device) = 0;
    virtual void Update(float dt) = 0;
    ID3D11Buffer* m_pModelBuffer = nullptr;
    ID3D11Buffer* m_pVertexBuffer = nullptr;
    ID3D11Buffer* m_pIndexBuffer = nullptr;
protected:

    ID3D11DeviceContext* m_context = nullptr;
    float m_rotationAngle = 0.0f;
};

