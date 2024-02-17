#include "header/ActivationFunctions.h"
#include <cmath>
#include <algorithm>

namespace ActivationFunctions
{
    double square(double x)
    {
        return x * x;
    }

    double sigmoid(double x)
    {
        return 1.0 / (1.0 + std::exp(-x));
    }

    double relu(double x)
    {
        return (x > 0) ? x : 0;
    }
};