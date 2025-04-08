#pragma once
#include "Cube.h"
#include "Pyramid.h"

class ModelFactory {
public:
    enum ModelCode {
        cube,
        pyramid,
    };
    static ModelManagerAbstract* CreateModel(ModelCode code, ID3D11DeviceContext* context) {
        switch (code) {
        case ModelCode::cube:
            return new Cube(context);
        case ModelCode::pyramid:
            return new Pyramid(context);
        default:
            return nullptr;
        }
    }

    static void ReleaseModel(ModelManagerAbstract* model) {
        if (model) delete model;
    }
};

