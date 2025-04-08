#pragma once
#include "ModelManagerAbstract.h"

class Pyramid : public ModelManagerAbstract
{
public:
    Pyramid(ID3D11DeviceContext* context);
    void Update(float dt) override;
    void Render() override;
    HRESULT InitModel(ID3D11Device* device) override;

private:
    float m_rotationAngle;
};

