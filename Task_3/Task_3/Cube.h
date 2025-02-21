#pragma once
#include "ModelManagerAbstract.h"

class Cube : public ModelManagerAbstract {
public:
    Cube(ID3D11Device* device) : ModelManagerAbstract(device) {
        // »нициализаци€ вершин и индексов куба
        Vertex vertices[] = {
            // ѕередн€€ грань
            {{-0.5f, -0.5f, -0.5f}, {1,0,0,1}},
            {{ 0.5f, -0.5f, -0.5f}, {1,1,0,1}},
            // ... все 8 вершин
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
    ~Cube() {};
    void Update(float dt) override;
    void Render(ID3D11DeviceContext* context) override;
};