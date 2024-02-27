#include <iostream>
#include <vector>
#include <algorithm>
#include "seal/seal.h"

using namespace std;
using namespace seal;

void printVector(const std::vector<double> &vec, const int limit)
{
    for (int i = 0; i < limit; i++)
    {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;
};

int main()
{
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
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    CKKSEncoder encoder(context);

    vector<double> modRow1{1.0, 3.0};
    vector<double> modRow2{2.0, 4.0};
    vector<vector<double>> matrix{{10, 11},
                                  {12, 13}};

    int resultDim = matrix[0].size();

    Plaintext encodedRow1, encodedRow2;
    encoder.encode(modRow1, scale, encodedRow1);
    encoder.encode(modRow2, scale, encodedRow2);

    Ciphertext inputCt[resultDim];
    Ciphertext resultCt[resultDim];
    Ciphertext encryptedRow1, encryptedRow2;
    encryptor.encrypt(encodedRow1, encryptedRow1);
    encryptor.encrypt(encodedRow2, encryptedRow2);

    inputCt[0] = encryptedRow1;
    inputCt[1] = encryptedRow2;

    Plaintext encodedWeight;
    int rowCnt = matrix.size();
    int colCnt = matrix[0].size();

    for (int i = 0; i < rowCnt; i++)
    {
        for (int j = 0; j < colCnt; j++)
        {
            Ciphertext mulResult, addResult;
            encoder.encode(matrix[i][j], scale, encodedWeight);
            evaluator.multiply_plain(inputCt[j], encodedWeight, mulResult);
            evaluator.rescale_to_next_inplace(mulResult);
            if (j == 0)
            {
                resultCt[i] = Ciphertext(mulResult);
            }
            else
            {
                evaluator.add(resultCt[i], mulResult, addResult);
                resultCt[i] = addResult;
            }
        }
    }

    Plaintext plainResult;
    vector<double> vectorResult;
    for (int i = 0; i < resultDim; i++)
    {
        decryptor.decrypt(resultCt[i], plainResult);
        encoder.decode(plainResult, vectorResult);
        printVector(vectorResult, resultDim);
    }
}