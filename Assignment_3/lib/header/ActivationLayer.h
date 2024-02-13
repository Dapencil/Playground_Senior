#pragma once

#include "Layer.h"
#include <vector>
#include <functional>
#include <string>

class ActivationLayer : public Layer
{
private:
    std::function<double(double)> activationFunction;
    std::string _name;

public:
    explicit ActivationLayer(std::function<double(double)> activation,
                             const std::string &functionName);
    ~ActivationLayer(){};

    std::vector<double> forward(const std::vector<double> &input) override;
    void printLayer() const override;
};