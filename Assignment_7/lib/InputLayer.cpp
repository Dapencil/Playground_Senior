// InputLayer.cpp
#include "header/InputLayer.h"
#include <stdexcept>
#include <iostream>

InputLayer::InputLayer(int inputSize) : inputDim(inputSize)
{
    if (inputSize <= 0)
    {
        throw std::invalid_argument("Input size must be positive");
    }
}

std::vector<double> InputLayer::forward(const std::vector<double> &input)
{
    if (input.size() != inputDim)
    {
        throw std::invalid_argument("Input size does not match the expected size");
    }

    // Input layer simply returns the input values unchanged
    return input;
}

void InputLayer::printLayer() const
{
    std::cout << "Input Layer ( " << inputDim << " )" << std::endl;
}