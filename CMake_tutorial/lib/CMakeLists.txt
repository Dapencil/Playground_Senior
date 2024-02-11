add_library(mylib mylib.cpp)

target_include_directories(mylib
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR})