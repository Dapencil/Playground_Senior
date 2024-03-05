// ActivationLayer.cpp
#include "header/ActivationLayer.h"
#include "header/Layer.h"
#include <stdexcept>
#include <cmath>
#include <iostream>

ActivationLayer::ActivationLayer(std::function<double(double)> activationFunction,
                                 const std::string &functionName)
    : activationFunction(activationFunction),
      _name(functionName)
{
    if (!activationFunction)
    {
        throw std::invalid_argument("Activation function must be provided");
    }
}

std::vector<double> ActivationLayer::forward(const std::vector<double> &input)
{
    // Apply the activation function element-wise to the input
    std::vector<double> output;
    output.reserve(input.size());

    for (double value : input)
    {
        output.push_back(activationFunction(value));
    }

    return output;
}

void ActivationLayer::printLayer() const
{

    std::cout << "Activation Layer ( " << _name << " )" << std::endl;
}
