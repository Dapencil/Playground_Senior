#pragma once

#include <vector>

class Layer
{
public:
    virtual std::vector<double> forward(const std::vector<double> &input);
    virtual ~Layer() = default;
};