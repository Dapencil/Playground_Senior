#pragma once

#include "Layer.h"
#include <vector>
#include <functional>

using namespace std;

class ActivationLayer : public Layer
{
private:
    function<double(double)> activationFunction;

public:
    explicit ActivationLayer(function<double(double)> activation);
    vector<double> forward(const vector<double> &input) override;
};