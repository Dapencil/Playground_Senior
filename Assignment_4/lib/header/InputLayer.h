// InputLayer.h
#pragma once

#include "Layer.h"

class InputLayer : public Layer
{
private:
    int inputDim;

public:
    InputLayer(int inputSize);
    ~InputLayer(){};

    std::vector<double> forward(const std::vector<double> &input) override;
    void printLayer() const;
};
