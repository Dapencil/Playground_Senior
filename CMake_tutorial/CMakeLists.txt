cmake_minimum_required(VERSION 3.6)
project(helloworld)

add_executable(${PROJECT_NAME} main.cpp)
find_package(fmt)
find_package(SEAL 4.1 REQUIRED)


target_link_libraries(${PROJECT_NAME} mylib fmt::fmt SEAL::seal)
add_subdirectory(lib)