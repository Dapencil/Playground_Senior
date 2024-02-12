#pragma once

#include <vector>

class Layer
{

public:
    virtual ~Layer(){};

    virtual std::vector<double> forward(const std::vector<double> &input) = 0;
    virtual void printLayer() const = 0;
};