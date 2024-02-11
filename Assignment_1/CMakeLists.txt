cmake_minimum_required(VERSION 3.6)
project(assign1)

add_executable(${PROJECT_NAME} main.cpp)
find_package(SEAL 4.1 REQUIRED)

target_link_libraries(${PROJECT_NAME} SEAL::seal)