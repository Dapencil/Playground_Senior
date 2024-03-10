cmake_minimum_required(VERSION 3.6)
project(assign7)

add_executable(${PROJECT_NAME} main.cpp bias_0.cpp weight_0.cpp)
find_package(SEAL 4.1 REQUIRED)

target_link_libraries(${PROJECT_NAME} SEAL::seal PROJECT_LIB)
add_subdirectory(lib)