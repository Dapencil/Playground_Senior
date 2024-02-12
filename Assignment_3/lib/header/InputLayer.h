// InputLayer.h
#pragma once

#include "Layer.h"

class InputLayer : public Layer
{
private:
    int inputDim;

public:
    explicit InputLayer(int inputSize);

    std::vector<double> forward(const std::vector<double> &input) override;
};
