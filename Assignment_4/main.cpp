#include "Util.h"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

vector<double> extractDiagonal(const vector<vector<double>> &matrix, size_t col)
{
    size_t maxRows = matrix.size();
    size_t maxCols = matrix[0].size();

    // Create a new vector to store the extracted diagonal
    // diag.size == matrix[0].size
    vector<double> diagonal;

    // Extract the diagonal starting from the specified row
    for (size_t row = 0; row < maxRows; ++row)
    {
        diagonal.push_back(matrix[row][(col + row) % maxCols]);
        // diagonal.push_back(matrix[(startingRow + i) % size][i]);
    }

    return diagonal;
}

vector<double> elemtwiseMul(const vector<double> &vec1, const vector<double> &vec2)
{
    // Check if vectors have the same size
    if (vec1.size() != vec2.size())
    {
        cerr << "Error: Vectors must have the same size for element-wise addition." << endl;
        return vector<double>();
    }

    vector<double> result(vec1.size());
    for (size_t i = 0; i < vec1.size(); ++i)
    {
        result[i] = vec1[i] * vec2[i];
    }

    return result;
}

vector<double> elementwiseAdd(const vector<double> &vec1, const vector<double> &vec2)
{
    // Check if vectors have the same size
    if (vec1.size() != vec2.size())
    {
        cerr << "Error: Vectors must have the same size for element-wise addition." << endl;
        return vector<double>();
    }

    // Create a result vector with the same size as the input vectors
    vector<double> result(vec1.size());

    // Perform element-wise addition
    for (size_t i = 0; i < vec1.size(); ++i)
    {
        result[i] = vec1[i] + vec2[i];
    }

    return result;
}

int main()
{
    vector<double> v{1.0, 2.0, 3.0};
    vector<vector<double>> matrix{{1.0, 2.0, 3.0},
                                  {4.0, 5.0, 6.0},
                                  {7.0, 8.0, 9.0}};

    vector<double> dest(v.size());
    vector<double> result(matrix.size());
    // std::rotate_copy(v.begin(), v.begin() + 1, v.end(), dest.begin());
    // printVector(v);
    // printVector(dest);
    // for (int i = 0; i < v.size(); i++)
    // {
    //     rotate_copy(v.begin(), v.begin() + i, v.end(), dest.begin());
    //     printVector(dest);
    // }
    for (int i = 0; i < matrix[0].size(); i++)
    {
        vector<double> diagWeight;
        vector<double> mulResult;
        rotate_copy(v.begin(),
                    v.begin() + i,
                    v.end(),
                    dest.begin());
        cout << "Rotated input" << endl;
        printVector(dest);
        cout << "diag weight" << endl;
        diagWeight = extractDiagonal(matrix, i);
        printVector(diagWeight);
        cout << "Elem-wise mul" << endl;
        mulResult = elemtwiseMul(diagWeight, dest);
        printVector(mulResult);
        cout << "Total Result" << endl;
        result = elementwiseAdd(result, mulResult);
    }
    printVector(result);
}