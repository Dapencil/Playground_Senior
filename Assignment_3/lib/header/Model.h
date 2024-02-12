#pragma once

#include "Layer.h"
#include <vector>

class Model
{
private:
    std::vector<Layer *> layers;

public:
    ~Model();

    void addLayer(Layer *layer);
    std::vector<double> forward(const std::vector<double> &input);
    void printModel() const;
};