cmake_minimum_required(VERSION 3.12)
project(Matching_Engine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)

# Add the executable target and specify the source files
add_executable(Matching_Engine
  src/main.cpp
  src/book_builder.cpp
  src/order_management_system.cpp
)

# Add the include directories
target_include_directories(Matching_Engine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/includes
)