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
    ~LinearLayer(){};
    LinearLayer(int inputSize,
                int outputSize);

    LinearLayer(vector<vector<double>> weights,
                vector<double> biases);

    vector<double> forward(const vector<double> &inputVector) override;
    void printLayer() const override;
    void printWeight() const;
    void printBias() const;
};