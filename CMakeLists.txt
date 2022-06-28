
cmake_policy(VERSION 3.23)
project(stack_exposures VERSION 1.0.0)

cmake_minimum_required(VERSION 3.23)

add_executable(my_prog src/my_prog.cpp)
target_compile_features(my_prog PUBLIC cxx_std_20)
