#pragma once

#include "Layer.h"
#include <vector>

using namespace std;
class Model
{
private:
    vector<Layer *> layers;

public:
    ~Model();

    void addLayer(Layer *layer);
    vector<double> forward(const vector<double> &input);
};