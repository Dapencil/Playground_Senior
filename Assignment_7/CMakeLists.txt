cmake_minimum_required(VERSION 3.6)
project(assign7)

add_executable(${PROJECT_NAME} 
main.cpp 
bias_0.cpp 
weight_0.cpp
bias_2.cpp 
weight_2.cpp
bias_4.cpp 
weight_4.cpp
bias_6.cpp 
weight_6.cpp)
find_package(Protobuf REQUIRED)
find_package(SEAL 4.1 REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} SEAL::seal protobuf::libprotobuf PROJECT_LIB)
add_subdirectory(lib)