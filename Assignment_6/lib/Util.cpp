#include "header/Util.h"
#include <vector>
#include <iostream>

void printVector(const std::vector<double> &vec, size_t limit)
{
    for (size_t i = 0; i < limit; i++)
    {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;
};