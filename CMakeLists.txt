cmake_minimum_required(VERSION 3.6)

project(clangmetatool CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Clang REQUIRED)

add_library(
  clangmetatool

  src/include_graph/include_finder.cpp
  src/include_graph/find_type_match_callback.cpp
  src/include_graph/find_decl_match_callback.cpp
  src/include_graph/find_decl_ref_match_callback.cpp
  src/include_graph/include_graph_util.cpp
  src/include_graph/include_graph.cpp

  src/variable_refs/variable_refs.cpp

  src/member_method_decls/member_method_decls.cpp

  src/find_calls/find_calls.cpp
)
target_include_directories(
  clangmetatool
  PRIVATE ${CLANG_INCLUDE_DIRS}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${LLVM_INCLUDE_DIR}>
)
target_link_libraries(
  clangmetatool
  clangTooling
)

install(
  TARGETS clangmetatool EXPORT clangmetatool
  LIBRARY DESTINATION ${LLVM_LIBRARY_DIR}
  ARCHIVE DESTINATION ${LLVM_LIBRARY_DIR}
  PUBLIC_HEADER DESTINATION ${LLVM_INCLUDE_DIR}
)
install(
  DIRECTORY include/
  DESTINATION ${LLVM_INCLUDE_DIR}
)
install(
  EXPORT clangmetatool
  DESTINATION ${CLANG_CMAKE_DIR}
)
install(
  FILES
   cmake/clangmetatool-find-clang-include-dir.pl
  DESTINATION ${CLANG_CMAKE_DIR}
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_WRITE GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)
install(
  FILES
   cmake/clangmetatool-config.cmake
  DESTINATION ${CLANG_CMAKE_DIR}
)

enable_testing()

if (NOT GTEST_IS_INCOMPATIBLE)
   add_subdirectory(t)
endif()
