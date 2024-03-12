add_library(PROJECT_LIB 
    Util.cpp
    InputLayer.cpp
    LinearLayer.cpp
    ActivationLayer.cpp
    ActivationFunctions.cpp
    Model.cpp
    weights.pb.cc
)

target_include_directories(PROJECT_LIB INTERFACE header)