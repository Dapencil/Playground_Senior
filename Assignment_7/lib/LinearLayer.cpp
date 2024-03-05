// LinearLayer.cpp
#include "header/LinearLayer.h"
#include "header/Util.h"
#include <cstdlib>
#include <stdexcept>
#include <iostream>

LinearLayer::LinearLayer(int inputSize,
                         int outputSize)
{
    if (inputSize <= 0 || outputSize <= 0)
    {
        throw std::invalid_argument("Input and output sizes must be positive");
    }

    inputDim = inputSize;
    outputDim = outputSize;

    // Initialize weights and biases (for simplicity, using random initialization)
    // You would typically use better initialization methods
    _weights.resize(outputSize, std::vector<double>(inputSize));
    for (auto &row : _weights)
    {
        for (auto &weight : row)
        {
            weight = ((double)rand() / RAND_MAX) - 0.5; // Random value between -0.5 and 0.5
        }
    }

    _biases.resize(outputSize);
    for (auto &bias : _biases)
    {
        // TODO: This is psudo random. it is fixed at complie time
        bias = ((double)rand() / RAND_MAX) - 0.5; // Random value between -0.5 and 0.5
    }
}

LinearLayer::LinearLayer(
    std::vector<std::vector<double>> weights,
    std::vector<double> biases) : _weights(weights),
                                  _biases(biases)
{
    if (_weights.empty() || _biases.empty() || _weights[0].size() != _biases.size())
    {
        throw std::invalid_argument("Invalid input weights or biases");
    }
    inputDim = _weights.size(); // row counts
    outputDim = _biases.size();
}

std::vector<double> LinearLayer::forward(const std::vector<double> &input)
{
    // Perform linear transformation: output = weights * input + biases
    std::vector<double> output(_weights.size());

    for (size_t i = 0; i < _weights.size(); ++i)
    {
        for (size_t j = 0; j < _weights[i].size(); ++j)
        {
            output[i] += _weights[i][j] * input[j];
        }
        output[i] += _biases[i];
    }

    return output;
}

void LinearLayer::printLayer() const
{
    std::cout << "Linear Layer ( " << inputDim << ", " << outputDim << " )" << std::endl;
}

void LinearLayer::printWeight() const
{
}

void LinearLayer::printBias() const
{
    std::cout << "Bias ";
    // printVector(_biases);
}