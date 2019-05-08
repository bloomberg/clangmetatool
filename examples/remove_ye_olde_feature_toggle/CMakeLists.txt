cmake_minimum_required(VERSION 3.6)

project( remove_ye_olde_feature_toggle C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Clang REQUIRED)
find_package(clangmetatool REQUIRED)

add_executable(
  remove_ye_olde_feature_toggle
  src/main.cpp
  # add other source names here
)

target_include_directories(remove_ye_olde_feature_toggle PRIVATE ${CLANG_INCLUDE_DIRS} )
target_link_libraries(remove_ye_olde_feature_toggle clangmetatool clangTooling )

clangmetatool_install(remove_ye_olde_feature_toggle)

enable_testing()
add_subdirectory(t)
