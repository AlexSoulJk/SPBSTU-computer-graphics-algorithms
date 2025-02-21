#pragma once
#include "Cube.h"
#include "Triangle.h"




class ModelFactory {
public:
    enum ModelCode {
        cube,
        triangle,
    };
    static ModelManagerAbstract* CreateModel(ModelCode code, ID3D11Device* device) {
        switch (code) {
        case ModelCode::cube:
            return new Cube(device);
        case ModelCode::triangle:
            return new Triangle(device);
        default:
            return nullptr;
        }
    }

    static void ReleaseModel(ModelManagerAbstract* model) {
        if (model) delete model;
    }
};

