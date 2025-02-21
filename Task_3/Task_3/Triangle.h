#pragma once
#include "ModelManagerAbstract.h"

class Triangle : public ModelManagerAbstract
{
public:
    Triangle(ID3D11Device* device) : ModelManagerAbstract(device) {
        // »нициализаци€ вершин и индексов треугольничка
        const Vertex vertices[] = {
        { {0.0f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
        { {0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f} }
        };
        WORD indices[] = {
            0,1,2, 2,3,0, // ѕередн€€
            // ... остальные грани
        };

        // —оздание буферов
        D3D11_BUFFER_DESC vbDesc = { 0 };
        vbDesc.ByteWidth = sizeof(vertices);
        vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA vbData = { vertices };
        m_device->CreateBuffer(&vbDesc, &vbData, &m_vertexBuffer);
    }
    ~Triangle() {};
    void Update(float dt) override;
    void Render(ID3D11DeviceContext* context) override;
};

