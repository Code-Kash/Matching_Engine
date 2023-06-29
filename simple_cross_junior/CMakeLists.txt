# Define the minimum version of cmake required
cmake_minimum_required(VERSION 3.10)

# Define your project name
project(SimpleCross)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Add an executable with the above sources
add_executable(SimpleCross simple_cross.cpp)