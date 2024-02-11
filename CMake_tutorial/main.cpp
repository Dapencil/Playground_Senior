#include "mylib.h"
#include <fmt/core.h>
#include "seal/seal.h"

using namespace seal;

int main()
{
    printHW();

    fmt::print("External Lib\n");

    EncryptionParameters params(scheme_type::bfv);

    return 0;
}