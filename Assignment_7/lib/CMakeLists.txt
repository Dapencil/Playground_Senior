add_library(PROJECT_LIB 
    Util.cpp
    InputLayer.cpp
    LinearLayer.cpp
    ActivationLayer.cpp
    ActivationFunctions.cpp
    Model.cpp
    global.cpp
)

target_include_directories(PROJECT_LIB INTERFACE header)