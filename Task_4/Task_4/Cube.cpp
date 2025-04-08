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
        { {-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f} },
        { {1.0f, -1.0f,  1.0f}, {1.0f, 1.0f} },
        { {1.0f, -1.0f, -1.0f}, {1.0f, 0.0f} },
        { {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f} },

        { {-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f} },
        { {1.0f,  1.0f, -1.0f}, {1.0f, 1.0f} },
        { {1.0f,  1.0f,  1.0f}, {1.0f, 0.0f} },
        { {-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f} },

        { {1.0f, -1.0f, -1.0f}, {0.0f, 1.0f} },
        { {1.0f, -1.0f,  1.0f}, {1.0f, 1.0f} },
        { {1.0f,  1.0f,  1.0f}, {1.0f, 0.0f} },
        { {1.0f,  1.0f, -1.0f}, {0.0f, 0.0f} },

        { {-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f} },
        { {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f} },
        { {-1.0f,  1.0f, -1.0f}, {1.0f, 0.0f} },
        { {-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f} },

        { {1.0f, -1.0f,  1.0f}, {0.0f, 1.0f} },
        { {-1.0f, -1.0f,  1.0f}, {1.0f, 1.0f} },
        { {-1.0f,  1.0f,  1.0f}, {1.0f, 0.0f} },
        { {1.0f,  1.0f,  1.0f}, {0.0f, 0.0f} },

        { {-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f} },
        { {1.0f, -1.0f, -1.0f}, {1.0f, 1.0f} },
        { {1.0f,  1.0f, -1.0f}, {1.0f, 0.0f} },
        { {-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f} }
    };

    WORD indices[] =
    {
        0, 2, 1,
        0, 3, 2,

        4, 6, 5,
        4, 7, 6,

        8, 10, 9,
        8, 11, 10,

        12, 14, 13,
        12, 15, 14,

        16, 18, 17,
        16, 19, 18,

        20, 22, 21,
        20, 23, 22
    };

    // �������� �������
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