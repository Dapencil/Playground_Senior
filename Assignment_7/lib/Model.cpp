// Model.cpp
#include "header/Model.h"
#include <iostream>

Model::~Model()
{
    for (Layer *layer : layers)
    {
        delete layer;
    }
}

void Model::addLayer(Layer *layer)
{
    layers.push_back(layer);
}

std::vector<double> Model::forward(const std::vector<double> &input)
{
    std::vector<double> output = input;

    for (Layer *layer : layers)
    {
        output = layer->forward(output);
    }

    return output;
}

void Model::printModel() const
{
    std::cout << "Model: " << std::endl;

    for (size_t i = 0; i < layers.size(); ++i)
    {
        std::cout << "Layer " << i << ": ";
        // Assuming each layer has a print function
        layers[i]->printLayer();
    }
}
