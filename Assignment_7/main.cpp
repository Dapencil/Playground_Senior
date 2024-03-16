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
    Weight weights;

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

vector<double> readBinaryBiasFile(const string &filename, int numCols)
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
    Bias bias;

    if (!bias.ParseFromString(serialized_data))
    {
        cerr << "Failed to parse serialized data." << std::endl;
        return {};
    }
    vector<double> bias_data;

    for (int i = 0; i < numCols; ++i)
    {
        bias_data.push_back(bias.values(i));
    }

    return bias_data;
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
                               const Evaluator &evaluator_,
                               const RelinKeys &relin_keys)
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
                evaluator_.relinearize_inplace(output_vector[i], relin_keys);
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
                   const RelinKeys &relin_keys,
                   size_t poly_modulus_degree,
                   const vector<int> &bits_of_coff)
        : public_key_(public_key),
          relin_keys_(relin_keys),
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
            cout << "Start Forward layer.." << endl;
            auto start = chrono::high_resolution_clock::now();
            current_input = layer.forward(current_input, encoder_, evaluator_, relin_keys_);
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::minutes>(end - start);
            cout << "Lyaer Execution time: " << duration.count() << " minutes" << endl;
        }
        return current_input;
    }

private:
    SEALContext context_;
    PublicKey public_key_;
    RelinKeys relin_keys_;
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
    // size_t poly_modulus_degree = 32768;

    vector<int> bits_of_coeff{60, 40, 40, 40, 40, 40, 40, 40, 60};
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, bits_of_coeff));
    double scale = pow(2.0, 40);

    SEALContext context(parms);
    KeyGenerator keygen(context);

    SecretKey secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    CKKSEncoder encoder(context);

    // TODO: 1. Batching Input 2. Encrypt Input
    //                              a    b
    vector<vector<double>> inputs;
    for (int i = 0; i < 784; ++i)
    {
        inputs.push_back({1.0});
    }
    vector<Ciphertext> encrypted_inputs;
    size_t batch_size = 1;

    Plaintext encoded_row;
    Ciphertext encrypted_row;
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        encoder.encode(inputs[i], scale, encoded_row);
        encryptor.encrypt(encoded_row, encrypted_row);
        encrypted_inputs.push_back(encrypted_row);
    }

    // Provider Section
    // vector<vector<double>> weight_00 = readBinaryWeightFile("../weight_00.bin", 1024, 784);
    // vector<double> bias_00 = readBinaryBiasFile("../bias_00.bin", 1024);
    // CKKSDense layer_0 = CKKSDense(weight_00, bias_00);

    /*
        vector<vector<double>> weight_02 = readBinaryWeightFile("../weight_02.bin", 1024, 1024);
        vector<double> bias_02 = readBinaryBiasFile("../bias_02.bin", 1024);
        CKKSDense layer_2 = CKKSDense(weight_02, bias_02);

        vector<vector<double>> weight_04 = readBinaryWeightFile("../weight_04.bin", 512, 1024);
        vector<double> bias_04 = readBinaryBiasFile("../bias_04.bin", 512);
        CKKSDense layer_4 = CKKSDense(weight_04, bias_04);

        vector<vector<double>> weight_06 = readBinaryWeightFile("../weight_06.bin", 10, 512);
        vector<double> bias_06 = readBinaryBiasFile("../bias_06.bin", 10);
        CKKSDense layer_6 = CKKSDense(weight_06, bias_06, false);
        */
    extern vector<vector<double>> weight_0;
    extern vector<double> bias_0;
    CKKSDense layer_0 = CKKSDense(weight_0, bias_0);

    extern vector<vector<double>> weight_2;
    extern vector<double> bias_2;
    CKKSDense layer_2 = CKKSDense(weight_2, bias_2);

    extern vector<vector<double>> weight_4;
    extern vector<double> bias_4;
    CKKSDense layer_4 = CKKSDense(weight_4, bias_4);

    extern vector<vector<double>> weight_6;
    extern vector<double> bias_6;
    CKKSDense layer_6 = CKKSDense(weight_6, bias_6, false);

    EncryptedModel model = EncryptedModel(public_key, relin_keys, poly_modulus_degree, bits_of_coeff);

    model.addLayer(layer_0);
    model.addLayer(layer_2);
    model.addLayer(layer_4);
    model.addLayer(layer_6);

    cout << layer_0.get_input_size() << "," << layer_0.get_output_size() << endl;
    auto start = chrono::high_resolution_clock::now();
    cout << "Start computing.." << endl;
    vector<Ciphertext> result_vector = model.predict(encrypted_inputs);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::minutes>(end - start);
    cout << "Total Execution time: " << duration.count() << " minutes" << endl;

    // User Section
    Plaintext decrypted_row_result;
    vector<double> decoded_row_result;
    int result_dim = 10;
    for (size_t i = 0; i < result_dim; ++i)
    {
        decryptor.decrypt(result_vector[i], decrypted_row_result);
        encoder.decode(decrypted_row_result, decoded_row_result);
        printVector(decoded_row_result, batch_size);
    }
    /*
        Plaintext decrypted_input_result;
        vector<double> decoded_input_result;
        int input_dim = 784;
        for (size_t i = 0; i < input_dim; ++i)
        {
            decryptor.decrypt(encrypted_inputs[i], decrypted_input_result);
            encoder.decode(decrypted_input_result, decoded_input_result);
            printVector(decoded_input_result, batch_size);
        }
    */
}