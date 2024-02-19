#include "Util.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include "seal/seal.h"

using namespace std;
using namespace seal;

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
    vector<double> v{500.0, 500.0, 500.0};
    vector<vector<double>> matrix{{1.0, 2.0, 3.0},
                                  {4.0, 5.0, 6.0},
                                  {7.0, 8.0, 9.0}};

    vector<double> dest(v.size());
    vector<double> result(matrix.size());

    // for (int i = 0; i < matrix[0].size(); i++)
    // {
    //     vector<double> diagWeight;
    //     vector<double> mulResult;
    //     rotate_copy(v.begin(),
    //                 v.begin() + i,
    //                 v.end(),
    //                 dest.begin());
    //     cout << "Rotated input" << endl;
    //     printVector(dest);
    //     cout << "diag weight" << endl;
    //     diagWeight = extractDiagonal(matrix, i);
    //     printVector(diagWeight);
    //     cout << "Elem-wise mul" << endl;
    //     mulResult = elemtwiseMul(diagWeight, dest);
    //     printVector(mulResult);
    //     cout << "Total Result" << endl;
    //     result = elementwiseAdd(result, mulResult);
    // }
    // printVector(result);

    EncryptionParameters parms(scheme_type::ckks);
    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, {60, 40, 40, 60}));
    double scale = pow(2.0, 40);

    SEALContext context(parms);
    KeyGenerator keygen(context);

    SecretKey secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);
    GaloisKeys galois_keys;
    keygen.create_galois_keys(galois_keys);

    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    CKKSEncoder encoder(context);

    Plaintext encodedInput, plainWeight;
    encoder.encode(v, scale, encodedInput);

    Ciphertext ct, rotatedCt, resultCt;
    encryptor.encrypt(encodedInput, ct);

    // size_t maxCol = matrix[0].size();
    // for (int i = 0; i < maxCol; i++)
    // {
    //     Ciphertext mulCt;
    //     encoder.encode(extractDiagonal(matrix, i), scale, plainWeight);
    //     evaluator.multiply_plain(ct, plainWeight, mulCt);
    //     evaluator.rescale_to_next_inplace(mulCt);

    //     if (i == 0)
    //     {
    //         resultCt = Ciphertext(mulCt);
    //     }
    //     else
    //     {
    //         evaluator.add_inplace(resultCt, mulCt);
    //     }
    // }
    Ciphertext rotated;
    evaluator.rotate_vector(ct, 2, galois_keys, rotated);

    Plaintext plainResult, plainRotated;
    decryptor.decrypt(ct, plainResult);
    decryptor.decrypt(rotated, plainRotated);

    vector<double> vectorResult, rotatedResult;
    encoder.decode(plainResult, vectorResult);
    encoder.decode(plainRotated, rotatedResult);

    cout << "==== Result ====" << endl;
    printVector(vectorResult);
    cout << "\n";
    printVector(rotatedResult);
}