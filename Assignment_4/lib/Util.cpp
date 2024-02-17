#include "header/Util.h"
#include <vector>
#include <iostream>

void printVector(const std::vector<double> &vec)
{
    for (int num : vec)
    {
        std::cout << num << " ";
    }
    std::cout << std::endl;
};