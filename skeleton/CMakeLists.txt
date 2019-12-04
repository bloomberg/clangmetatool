cmake_minimum_required(VERSION 3.6)

project( example C CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Clang REQUIRED)
find_package(clangmetatool REQUIRED)

add_executable(
  example
  src/main.cpp
  # add other source names here
)

target_include_directories(example PRIVATE ${CLANG_INCLUDE_DIRS} )
target_link_libraries(example clangTooling)

clangmetatool_install(example)

