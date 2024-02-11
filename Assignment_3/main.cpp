#include <iostream>
#include <vector>
#include <functional>

using namespace std;

vector<int> matrixVectorMultiplication(const vector<vector<int>> &matrix,
                                       const vector<int> &vec)
{
    int n = matrix.size();
    int m = matrix[0].size();
    if (vec.size() != m)
    {
        throw invalid_argument("Matrix's col must be equal to Vector's row for multiplication");
    }
    // nxm X mx1 = nx1
    vector<int> result(n, 0);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            result[i] += vec[j] * matrix[i][j];
        }
    }
    return result;
}

vector<int> addVector(const vector<int> &vecA,
                      const vector<int> &vecB)
{
    if (vecA.size() != vecB.size())
    {
        throw invalid_argument("Vectors must have the same dimesion for addition.");
    }
    int n = vecA.size();
    vector<int> result(n);

    for (int i = 0; i < n; i++)
    {
        result[i] = vecA[i] + vecB[i];
    }

    return result;
}

vector<int> applyToVector(const vector<int> &vec,
                          const function<int(int)> &f)
{
    int dim = vec.size();
    vector<int> result(dim);

    for (int i = 0; i < dim; i++)
    {
        result[i] = f(vec[i]);
    }
    return result;
}

int square(int x)
{
    return x * x;
}

void printVector(const vector<int> &vec)
{
    for (int num : vec)
    {
        cout << num << " ";
    }
    cout << endl;
}

int main()
{
    cout << "Assignment 2" << endl;
    vector<vector<int>> matrix = {{1, 2, 3},
                                  {4, 5, 6}};
    vector<int> vec = {7, 8, 9};
    vector<int> result = matrixVectorMultiplication(matrix, vec);
    vector<int> result2 = applyToVector(vec, square);
    printVector(result);
    // printVector(addVector(result, vec));
    printVector(result2);

    return 0;
}