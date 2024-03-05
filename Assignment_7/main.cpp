#include <iostream>
#include <vector>
#include "Util.h"
#include "seal/seal.h"

using namespace std;
using namespace seal;

class CKKSDense
{
public:
    // Constructor
    CKKSDense(vector<vector<double>> weights,
              vector<double> biases,
              const PublicKey &public_key)
        : public_key_(public_key),
          context_(create_context()),
          evaluator_(context_),
          encoder_(context_),
          weights_(weights),
          biases_(biases),
          input_size_(weights[0].size()),
          output_size_(weights.size()),
          scale_(pow(2.0, 40))
    {
    }

    vector<Ciphertext> forward(const vector<Ciphertext> &input)
    {

        vector<Ciphertext> dunno;
        Ciphertext output_vector[output_size_];
        Plaintext encoded_weight, encoded_bias;
        size_t rowCnt = output_size_;
        size_t colCnt = input_size_;

        // matrix multiplication
        for (size_t i = 0; i < rowCnt; i++)
        {
            for (size_t j = 0; j < colCnt; j++)
            {
                Ciphertext mul_result, add_result;
                encoder_.encode(weights_[i][j], scale_, encoded_weight);
                evaluator_.multiply_plain(input[j], encoded_weight, mul_result);
                evaluator_.rescale_to_next_inplace(mul_result);
                if (j == 0)
                {
                    output_vector[i] = Ciphertext(mul_result);
                }
                else
                {
                    evaluator_.add(output_vector[i], mul_result, add_result);
                    output_vector[i] = add_result;
                }
            }
            // add bias
            // Ciphertext add_bias_result;
            // encoder_.encode(biases_[i], scale_, encoded_bias);
            // evaluator_.add_plain(output_vector[i], encoded_bias, add_bias_result);
            // output_vector[i] = add_bias_result;
            dunno.push_back(output_vector[i]);
        }
        return dunno;
    }

private:
    SEALContext context_;
    // didn't use yet
    PublicKey public_key_;
    Evaluator evaluator_;
    CKKSEncoder encoder_;
    double scale_;

    size_t input_size_;
    size_t hidden_size_;
    size_t output_size_;

    vector<vector<double>> weights_;
    vector<double> biases_;

    SEALContext create_context()
    {
        EncryptionParameters parms(scheme_type::ckks);
        size_t poly_modulus_degree = 8192;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, {60, 40, 40, 60}));
        double scale = pow(2.0, 40);
        return SEALContext(parms);
    }
};

int main()
{
    // User Section
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
    // TODO: 1. Batching Iuput 2. Encrypt Input
    //                              a    b
    vector<vector<double>> inputs{{1.0, 1.0},
                                  {1.0, 1.0}};
    vector<Ciphertext> encrypted_inputs;
    size_t batch_size = 2;

    Plaintext encoded_row;
    Ciphertext encrypted_row;
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        encoder.encode(inputs[i], scale, encoded_row);
        encryptor.encrypt(encoded_row, encrypted_row);
        encrypted_inputs.push_back(encrypted_row);
    }

    // Provider Section
    vector<vector<double>> weights{{2.0, 2.0},
                                   {2.0, 2.0},
                                   {2.0, 2.0}};
    vector<double> biases{1.0, 1.0, 1.0};
    CKKSDense layer_1 = CKKSDense(weights, biases, public_key);
    vector<Ciphertext> result_vector = layer_1.forward(encrypted_inputs);

    // User Section
    Plaintext decrypted_row_result;
    vector<double> decoded_row_result;
    int result_dim = 3;
    for (size_t i = 0; i < result_dim; ++i)
    {
        decryptor.decrypt(result_vector[i], decrypted_row_result);
        encoder.decode(decrypted_row_result, decoded_row_result);
        printVector(decoded_row_result, batch_size);
    }
}