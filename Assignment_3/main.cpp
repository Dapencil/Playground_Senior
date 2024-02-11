#include "seal/seal.h"

using namespace std;
using namespace seal;

void printVector(const vector<double> &vec)
{
    for (int num : vec)
    {
        cout << num << " ";
    }
    cout << endl;
}

int main()
{
    // Initialize Microsoft SEAL context, keys, encryptor, evaluator, decryptor, etc.
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

    // Sample vectors
    vector<double> vectorA = {1.0, 2.0, 3.0};
    vector<double> vectorP_1 = {4.0, 4.0};
    vector<double> vectorP_2 = {5.0, 5.0};
    vector<double> vectorP_3 = {6.0, 6.0};

    // Encrypt matrices
    Plaintext plainVectorA, plainVectorP_1, plainVectorP_2, plainVectorP_3;
    Plaintext plainVectorC1, plainVectorC2, plainVectorC3;
    CKKSEncoder encoder(context);

    encoder.encode(vectorA, scale, plainVectorA);
    encoder.encode(vectorP_1, scale, plainVectorP_1);
    encoder.encode(vectorP_2, scale, plainVectorP_2);
    encoder.encode(vectorP_3, scale, plainVectorP_3);
    encoder.encode(1.0, scale, plainVectorC1);
    encoder.encode(2.0, scale, plainVectorC2);
    encoder.encode(3.0, scale, plainVectorC3);

    Ciphertext encryptedVectorA, encryptedVectorC_1, encryptedVectorC_2, encryptedVectorC_3;
    encryptor.encrypt(plainVectorA, encryptedVectorA);
    encryptor.encrypt(plainVectorC1, encryptedVectorC_1);
    encryptor.encrypt(plainVectorC2, encryptedVectorC_2);
    encryptor.encrypt(plainVectorC3, encryptedVectorC_3);

    // Perform multiplication on encrypted matrices
    Ciphertext encryptedAP_1, encryptedAP_2, encryptedAP_3;
    evaluator.multiply_plain(encryptedVectorC_1, plainVectorP_1, encryptedAP_1);
    evaluator.multiply_plain(encryptedVectorC_2, plainVectorP_2, encryptedAP_2);
    evaluator.multiply_plain(encryptedVectorC_3, plainVectorP_3, encryptedAP_3);

    // Rescale
    // evaluator.rescale_to_next_inplace(encryptedAP_1);
    // evaluator.rescale_to_next_inplace(encryptedAP_2);
    // evaluator.rescale_to_next_inplace(encryptedAP_3);

    // Add
    Ciphertext encryptedResult;
    evaluator.add(encryptedAP_1, encryptedAP_2, encryptedResult);
    evaluator.add(encryptedAP_3, encryptedResult, encryptedResult);

    // Decrypt the result
    Plaintext plainResult, plainAP_1, plainAP_2, plainAP_3;
    decryptor.decrypt(encryptedResult, plainResult);
    decryptor.decrypt(encryptedAP_1, plainAP_1);
    decryptor.decrypt(encryptedAP_2, plainAP_2);
    decryptor.decrypt(encryptedAP_3, plainAP_3);

    // Decode the result to obtain the matrix
    vector<double> result, resultAP_1, resultAP_2, resultAP_3;
    encoder.decode(plainResult, result);
    encoder.decode(plainAP_1, resultAP_1);
    encoder.decode(plainAP_2, resultAP_2);
    encoder.decode(plainAP_3, resultAP_3);

    // Display the result
    cout << "Decrypted" << endl;
    printVector(result);
    // printVector(resultAP_1);
    // printVector(resultAP_2);
    // printVector(resultAP_3);

    return 0;
}
