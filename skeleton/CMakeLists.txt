cmake_minimum_required(VERSION 3.6)

project( yourtoolname C CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Clang REQUIRED)
find_package(clangmetatool REQUIRED)

add_executable(
  yourtoolname
  src/main.cpp
  # add other source names here
)

target_include_directories(yourtoolname PRIVATE ${CLANG_INCLUDE_DIRS} )
target_link_libraries(yourtoolname clangTooling)

clangmetatool_install(yourtoolname)

