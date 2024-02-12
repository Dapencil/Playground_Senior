#pragma once

#include "Layer.h"

using namespace std;

class LinearLayer : public Layer
{
private:
    int inputDim;
    int outputDim;
    vector<vector<double>> _weights;
    vector<double> _biases;

public:
    LinearLayer(int inputSize,
                int outputSize,
                vector<vector<double>> weights,
                vector<double> biases);

    vector<double> forward(const vector<double> &inputVector) override;
};