#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "Util.h"
#include "seal/seal.h"
#include "weights.pb.h"

using namespace std;
using namespace seal;

vector<vector<double>> readBinaryWeightFile(const string &filename, int numRows, int numCols)
{
    ifstream file(filename, ios::binary);
    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return {};
    }

    // Read the binary data into a flat vector of doubles
    string serialized_data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    Weights weights;

    if (!weights.ParseFromString(serialized_data))
    {
        cerr << "Failed to parse serialized data." << std::endl;
        return {};
    }
    vector<vector<double>> weight_data;

    for (int i = 0; i < numRows; ++i)
    {
        vector<double> row;
        for (int j = 0; j < numCols; ++j)
        {
            row.push_back(weights.values((i * (numCols - 1)) + j));
        }
        weight_data.push_back(row);
    }

    return weight_data;
}

class CKKSDense
{
public:
    // Constructor
    CKKSDense(vector<vector<double>> weights,
              vector<double> biases,
              bool is_apply_activation = true)
        : weights_(weights),
          biases_(biases),
          input_size_(weights[0].size()),
          output_size_(weights.size()),
          is_apply_activation_(is_apply_activation)
    {
    }

    size_t get_input_size() const { return input_size_; }
    size_t get_output_size() const { return output_size_; }

    vector<Ciphertext> forward(const vector<Ciphertext> &input,
                               const CKKSEncoder &encoder_,
                               const Evaluator &evaluator_)
    {

        vector<Ciphertext> result;
        Ciphertext output_vector[output_size_];
        Plaintext encoded_weight, encoded_bias;
        size_t rowCnt = output_size_;
        size_t colCnt = input_size_;

        // assume that every ct is at the same scale
        double before_mul_scale = input[0].scale();
        parms_id_type before_mul_parms_id = input[0].parms_id();

        // matrix multiplication
        for (size_t i = 0; i < rowCnt; i++)
        {
            for (size_t j = 0; j < colCnt; j++)
            {
                Ciphertext mul_result, add_result;
                encoder_.encode(weights_[i][j], before_mul_scale, encoded_weight);
                evaluator_.mod_switch_to_inplace(encoded_weight, before_mul_parms_id);
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
            Ciphertext add_bias_result;
            parms_id_type last_parms_id = output_vector[i].parms_id();
            double after_mul_scale = output_vector[i].scale();

            encoder_.encode(biases_[i], after_mul_scale, encoded_bias);
            evaluator_.mod_switch_to_inplace(encoded_bias, last_parms_id);
            evaluator_.add_plain(output_vector[i], encoded_bias, add_bias_result);
            output_vector[i] = add_bias_result;

            // activation function
            if (is_apply_activation_)
            {
                evaluator_.square_inplace(output_vector[i]);
                evaluator_.rescale_to_next_inplace(output_vector[i]);
            }

            result.push_back(output_vector[i]);
        }
        return result;
    }

private:
    size_t input_size_;
    size_t output_size_;

    vector<vector<double>> weights_;
    vector<double> biases_;

    bool is_apply_activation_;
};

class EncryptedModel
{
public:
    EncryptedModel(const PublicKey &public_key,
                   size_t poly_modulus_degree,
                   const vector<int> &bits_of_coff)
        : public_key_(public_key),
          context_(create_context(poly_modulus_degree, bits_of_coff)),
          evaluator_(context_),
          encoder_(context_)
    {
    }

    void addLayer(const CKKSDense &layer)
    {
        if (!layers_.empty() && layers_.back().get_output_size() != layer.get_input_size())
            throw invalid_argument("Input size does not match the expected size.");
        layers_.push_back(layer);
    }

    vector<Ciphertext> predict(const vector<Ciphertext> &input)
    {
        vector<Ciphertext> current_input = input;
        for (auto &layer : layers_)
        {
            current_input = layer.forward(current_input, encoder_, evaluator_);
        }
        return current_input;
    }

private:
    SEALContext context_;
    PublicKey public_key_;
    Evaluator evaluator_;
    CKKSEncoder encoder_;

    vector<CKKSDense> layers_;

    SEALContext create_context(size_t poly_modulus_degree, const vector<int> &bits_of_coeff)
    {
        EncryptionParameters parms(scheme_type::ckks);
        parms.set_poly_modulus_degree(poly_modulus_degree);
        // limit of multiplicative depth
        parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, bits_of_coeff));

        return SEALContext(parms);
    }
};

int main()
{
    // User Section
    EncryptionParameters parms(scheme_type::ckks);
    // size_t poly_modulus_degree = 8192;
    size_t poly_modulus_degree = 16384;
    vector<int> bits_of_coeff{60, 40, 40, 40, 60};
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, bits_of_coeff));
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
    // TODO: 1. Batching Input 2. Encrypt Input
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
    // vector<vector<double>> weights_1{{2.0, 2.0},
    //                                  {2.0, 2.0},
    //                                  {2.0, 2.0}};
    // vector<double> biases_1{1.0, 1.0, 1.0};
    // CKKSDense layer_1 = CKKSDense(weights_1, biases_1);

    vector<vector<double>> weights_2{{1.0, 1.0, 1.0}};
    vector<double> biases_2{0.0, 0.0, 0.0};
    CKKSDense layer_2 = CKKSDense(weights_2, biases_2, false);
    extern vector<vector<double>> weight_0;
    extern vector<double> bias_0;
    vector<vector<double>> weight_00 = readBinaryWeightFile("../weight_0.bin", 1024, 784);
    CKKSDense layer_0 = CKKSDense(weight_00, bias_0);
    cout << layer_0.get_input_size() << endl;
    cout << layer_0.get_output_size() << endl;

    printVector(weight_00[0], 784);

    EncryptedModel model = EncryptedModel(public_key, poly_modulus_degree, bits_of_coeff);
    // model.addLayer(layer_1);
    // model.addLayer(layer_2);
    model.addLayer(layer_0);

    /*
    vector<Ciphertext> result_vector = model.predict(encrypted_inputs);

    // User Section
    Plaintext decrypted_row_result;
    vector<double> decoded_row_result;
    int result_dim = 1;
    for (size_t i = 0; i < result_dim; ++i)
    {
        decryptor.decrypt(result_vector[i], decrypted_row_result);
        encoder.decode(decrypted_row_result, decoded_row_result);
        printVector(decoded_row_result, batch_size);
    }

    Plaintext decrypted_input_result;
    vector<double> decoded_input_result;
    int input_dim = 2;
    for (size_t i = 0; i < input_dim; ++i)
    {
        decryptor.decrypt(encrypted_inputs[i], decrypted_input_result);
        encoder.decode(decrypted_input_result, decoded_input_result);
        printVector(decoded_input_result, batch_size);
    }
    */
}